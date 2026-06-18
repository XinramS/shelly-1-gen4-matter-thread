/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include <esp_matter.h>
#include <esp_matter_client.h>
#include <app_priv.h>
#include <common_macros.h>

#include <lib/core/Optional.h>

#include <driver/gpio.h>
#include <driver/temperature_sensor.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <device.h>
#include <button_gpio.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;
extern uint16_t switch_endpoint_id;

// Shelly 1 Gen4 GPIO assignments
#define RELAY_GPIO          GPIO_NUM_5
#define SWITCH_INPUT_GPIO   GPIO_NUM_10
#define BUTTON_GPIO         GPIO_NUM_4

// Temperature protection
#define TEMP_TRIP_CELSIUS   75.0f
#define TEMP_SUSTAINED_MS   10000

static temperature_sensor_handle_t temp_sensor = NULL;
static int64_t over_temp_start_ms = 0;
static bool thermal_fault = false;

// Switch input queue
static QueueHandle_t switch_evt_queue = NULL;

static esp_err_t app_driver_relay_set_power(bool power);

static void app_driver_temp_check()
{
    float temp = 0;
    esp_err_t err = temperature_sensor_get_celsius(temp_sensor, &temp);
    if (err != ESP_OK) return;

    ESP_LOGD(TAG, "Board temp: %.1f C", temp);

    if (temp > TEMP_TRIP_CELSIUS) {
        if (over_temp_start_ms == 0) {
            over_temp_start_ms = esp_timer_get_time() / 1000;
        } else {
            int64_t elapsed = (esp_timer_get_time() / 1000) - over_temp_start_ms;
            if (elapsed >= TEMP_SUSTAINED_MS && !thermal_fault) {
                ESP_LOGE(TAG, "Thermal fault! Temp: %.1f C shutting off relay", temp);
                thermal_fault = true;
                app_driver_relay_set_power(false);
                uint32_t cluster_id = OnOff::Id;
                uint32_t attribute_id = OnOff::Attributes::OnOff::Id;
                esp_matter_attr_val_t val = esp_matter_bool(false);
                attribute::update(light_endpoint_id, cluster_id, attribute_id, &val);
            }
        }
    } else {
        over_temp_start_ms = 0;
        thermal_fault = false;
    }
}

static esp_err_t app_driver_relay_set_power(bool power)
{
    if (thermal_fault && power) {
        ESP_LOGW(TAG, "Thermal fault active relay turn on blocked");
        return ESP_ERR_INVALID_STATE;
    }
    ESP_LOGI(TAG, "Setting relay: %s", power ? "ON" : "OFF");
    return gpio_set_level(RELAY_GPIO, power ? 1 : 0);
}

static void IRAM_ATTR app_driver_switch_isr(void *arg)
{
    uint32_t gpio_num = SWITCH_INPUT_GPIO;
    xQueueSendFromISR(switch_evt_queue, &gpio_num, NULL);
}

// Invoked by the binding manager once the bound peer's CASE session is ready.
// This is where the queued command is actually put on the wire.
static void send_command_success_callback(void *context, const ConcreteCommandPath &command_path,
                                          const chip::app::StatusIB &status, TLVReader *response_data)
{
    ESP_LOGI(TAG, "Bound command sent successfully");
}

static void send_command_failure_callback(void *context, CHIP_ERROR error)
{
    ESP_LOGE(TAG, "Bound command send failed: %" CHIP_ERROR_FORMAT, error.Format());
}

// Unicast binding callback: build and send the invoke command to the bound peer.
static void app_driver_client_callback(client::peer_device_t *peer_device, client::request_handle_t *req_handle,
                                       void *priv_data)
{
    if (req_handle->type != esp_matter::client::INVOKE_CMD) {
        return;
    }
    if (req_handle->command_path.mClusterId != OnOff::Id) {
        ESP_LOGE(TAG, "Unsupported cluster 0x%08" PRIx32 " for bound command", req_handle->command_path.mClusterId);
        return;
    }
    // OnOff Toggle/On/Off take no fields, so the command data is an empty object.
    const char *command_data_str = "{}";
    client::interaction::invoke::send_request(NULL, peer_device, req_handle->command_path, command_data_str,
                                              send_command_success_callback, send_command_failure_callback,
                                              chip::NullOptional);
}

// Group binding callback: send the command to the bound group.
static void app_driver_client_group_callback(uint8_t fabric_index, client::request_handle_t *req_handle,
                                             void *priv_data)
{
    if (req_handle->type != esp_matter::client::INVOKE_CMD) {
        return;
    }
    if (req_handle->command_path.mClusterId != OnOff::Id) {
        ESP_LOGE(TAG, "Unsupported cluster 0x%08" PRIx32 " for bound group command", req_handle->command_path.mClusterId);
        return;
    }
    const char *command_data_str = "{}";
    client::interaction::invoke::send_group_request(fabric_index, req_handle->command_path, command_data_str);
}

static void app_driver_switch_task(void *arg)
{
    while (true) {
        uint32_t io_num;
        if (xQueueReceive(switch_evt_queue, &io_num, portMAX_DELAY)) {
            // 50ms debounce
            vTaskDelay(pdMS_TO_TICKS(50));

            // Drain any additional events during debounce
            while (xQueueReceive(switch_evt_queue, &io_num, 0) == pdTRUE) {}

            ESP_LOGI(TAG, "SW input changed. Sending Toggle command");
            client::request_handle_t req_handle;
            req_handle.type = esp_matter::client::INVOKE_CMD;
            req_handle.command_path.mClusterId = OnOff::Id;
            req_handle.command_path.mCommandId = OnOff::Commands::Toggle::Id;

            lock::ScopedChipStackLock lock(portMAX_DELAY);
            client::cluster_update(switch_endpoint_id, &req_handle);
            
            ESP_LOGI(TAG, "Switch input toggled");
        }
    }
}

static void app_driver_button_toggle_cb(void *arg, void *data)
{
    ESP_LOGI(TAG, "Toggle button pressed");
    uint16_t endpoint_id = light_endpoint_id;
    uint32_t cluster_id = OnOff::Id;
    uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

    attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

    esp_matter_attr_val_t val = esp_matter_invalid(NULL);
    attribute::get_val(attribute, &val);
    val.val.b = !val.val.b;
    attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == light_endpoint_id) {
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = app_driver_relay_set_power(val->val.b);
            }
        }
    }
    return err;
}

esp_err_t app_driver_light_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    attribute_t *attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err = app_driver_relay_set_power(val.val.b);

    return err;
}

app_driver_handle_t app_driver_light_init()
{
    // Initialize relay GPIO
    gpio_config_t relay_cfg = {
        .pin_bit_mask = (1ULL << RELAY_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&relay_cfg);
    gpio_set_level(RELAY_GPIO, 0);

    // Initialize internal temperature sensor
    temperature_sensor_config_t temp_cfg = {
        .range_min = 20,
        .range_max = 100,
    };
    temperature_sensor_install(&temp_cfg, &temp_sensor);
    temperature_sensor_enable(temp_sensor);

    // Start temperature monitoring task
    xTaskCreate([](void *arg) {
        while (true) {
            app_driver_temp_check();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    }, "temp_monitor", 2048, NULL, 5, NULL);

    // Initialize external switch input GPIO10
    // NOTE: GPIO_PULLUP_DISABLE required — Shelly PCB has external pull resistor on GPIO10.
    // Internal pullup fights the external circuit and prevents ISR from firing.
    switch_evt_queue = xQueueCreate(10, sizeof(uint32_t));
    xTaskCreate(app_driver_switch_task, "switch_task", 4096, NULL, 10, NULL);

    // Register the binding dispatch callbacks. Without these, client::cluster_update()
    // resolves the bound peer and opens a CASE session but never emits the command.
    client::set_request_callback(app_driver_client_callback, app_driver_client_group_callback, NULL);

    gpio_config_t switch_cfg = {
        .pin_bit_mask = (1ULL << SWITCH_INPUT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&switch_cfg);

    // ISR service may already be installed by Matter stack
    esp_err_t isr_err = gpio_install_isr_service(0);
    if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %d", isr_err);
    }

    esp_err_t handler_err = gpio_isr_handler_add(SWITCH_INPUT_GPIO, app_driver_switch_isr, NULL);
    if (handler_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add switch ISR handler: %d", handler_err);
    }

    return (app_driver_handle_t)1;
}

app_driver_handle_t app_driver_button_init()
{
    button_handle_t handle = NULL;
    const button_config_t btn_cfg = {0};

    // Hardcoded GPIO4 for Shelly 1 Gen4 device button
    const button_gpio_config_t btn_gpio_cfg = {
        .gpio_num = GPIO_NUM_4,
        .active_level = 0,
    };

    if (iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create button device");
        return NULL;
    }

    iot_button_register_cb(handle, BUTTON_PRESS_DOWN, NULL, app_driver_button_toggle_cb, NULL);
    return (app_driver_handle_t)handle;
}