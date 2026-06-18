//
// Copyright 2026 AUTOMATOUS.IO
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

// contact_sensor.cpp

#include "contact_sensor.h"

#include <app_priv.h>

#include <esp_log.h>
#include <esp_matter.h>
#include <esp_matter_core.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// BooleanState StateValue is read-only / internally managed, so it must be
// updated via the cluster setter, not attribute::update().
#include <clusters/boolean_state/integration.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "contact_sensor";

// Defined in app_main.cpp. The endpoint whose Boolean State StateValue
// attribute we update when the door position changes. Zero until the
// endpoint is created; edges before then are ignored.
extern uint16_t contact_sensor_endpoint_id;

// Shelly 1 Gen4 SW terminal, used here as the reed/limit switch input.
//
// HARDWARE NOTE: GPIO_PULLUP_DISABLE and GPIO_PULLDOWN_DISABLE are
// required. The Shelly PCB has an external pull resistor on GPIO10.
// Enabling either internal pull will fight the external circuit and
// prevent the ISR from firing. Do not "fix" this by adding a pull.
#define CONTACT_GPIO        GPIO_NUM_10
#define DEBOUNCE_MS         50
#define QUEUE_DEPTH         10
#define TASK_STACK_SIZE     4096
#define TASK_PRIORITY       10

// ISR-to-task communication queue. Written by the ISR (FromISR),
// read by the task. File-static so the ISR can reach it.
static QueueHandle_t contact_evt_queue = nullptr;

static void IRAM_ATTR contact_sensor_isr(void *arg)
{
    uint32_t gpio_num = CONTACT_GPIO;
    xQueueSendFromISR(contact_evt_queue, &gpio_num, nullptr);
}

bool contact_sensor_read_state(void)
{
    // Map the raw GPIO level to the Matter StateValue (true = closed).
    int level = gpio_get_level(CONTACT_GPIO);
    bool is_open = (level == (CONTACT_OPEN_IS_HIGH ? 1 : 0));
    return !is_open;
}

void contact_sensor_report(void)
{
    // The endpoint may not exist yet during early boot edges.
    if (contact_sensor_endpoint_id == 0) {
        return;
    }

    bool state_value = contact_sensor_read_state();

    // Runs outside the Matter task, so take the CHIP stack lock.
    // SetStateValue() updates the attribute and emits the report.
    esp_matter::lock::ScopedChipStackLock lock(portMAX_DELAY);
    auto *cluster = chip::app::Clusters::BooleanState::FindClusterOnEndpoint(contact_sensor_endpoint_id);
    if (cluster == nullptr) {
        ESP_LOGE(TAG, "BooleanState cluster not found on endpoint %u", contact_sensor_endpoint_id);
        return;
    }
    cluster->SetStateValue(state_value);
    ESP_LOGI(TAG, "Contact state updated: %s", state_value ? "CLOSED" : "OPEN");
}

static void contact_sensor_task(void *arg)
{
    while (true) {
        uint32_t io_num;
        if (xQueueReceive(contact_evt_queue, &io_num, portMAX_DELAY)) {
            // Debounce: wait, then drain any additional events that
            // queued during the debounce window. This collapses a noisy
            // mechanical edge into a single logical event, after which we
            // read the settled level.
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
            while (xQueueReceive(contact_evt_queue, &io_num, 0) == pdTRUE) {}

            contact_sensor_report();
        }
    }
}

esp_err_t contact_sensor_init(void)
{
    // Create the ISR-to-task communication queue and the task that
    // services it. The task must exist before the ISR can fire
    // (otherwise events queue up with no consumer), so we create them
    // in this order.
    contact_evt_queue = xQueueCreate(QUEUE_DEPTH, sizeof(uint32_t));
    xTaskCreate(contact_sensor_task, "contact_sensor", TASK_STACK_SIZE,
                nullptr, TASK_PRIORITY, nullptr);

    // Configure GPIO10 as edge-interrupt input. See HARDWARE NOTE above
    // for why both internal pulls are disabled.
    gpio_config_t contact_cfg = {
        .pin_bit_mask = (1ULL << CONTACT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&contact_cfg);

    // Install the GPIO ISR service. ESP_ERR_INVALID_STATE means it was
    // already installed by another module (the Matter stack or
    // iot_button via button.cpp), which is fine — we share a single
    // ISR service across the whole app.
    esp_err_t isr_err = gpio_install_isr_service(0);
    if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %d", isr_err);
    }

    esp_err_t handler_err = gpio_isr_handler_add(CONTACT_GPIO,
                                                 contact_sensor_isr, nullptr);
    if (handler_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add contact ISR handler: %d", handler_err);
    }

    ESP_LOGI(TAG, "Contact sensor initialized on GPIO%d", CONTACT_GPIO);
    return ESP_OK;
}
