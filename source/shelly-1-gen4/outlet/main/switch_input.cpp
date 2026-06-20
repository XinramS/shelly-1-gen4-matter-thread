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

// switch_input.cpp

#include "switch_input.h"

#include <esp_log.h>
#include <esp_matter.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "switch_input";

// Defined in app_main.cpp. Used to know which Matter endpoint's OnOff
// attribute to toggle when the switch input fires.
extern uint16_t relay_endpoint_id;

// Shelly 1 Gen4 wall switch input.
//
// HARDWARE NOTE: GPIO_PULLUP_DISABLE and GPIO_PULLDOWN_DISABLE are
// required. The Shelly PCB has an external pull resistor on GPIO10.
// Enabling either internal pull will fight the external circuit and
// prevent the ISR from firing. Do not "fix" this by adding a pull.
#define SWITCH_INPUT_GPIO   GPIO_NUM_10
#define DEBOUNCE_MS         50
#define QUEUE_DEPTH         10
#define TASK_STACK_SIZE     4096
#define TASK_PRIORITY       10

// ISR-to-task communication queue. Written by the ISR (FromISR),
// read by the task. File-static so the ISR can reach it.
static QueueHandle_t switch_evt_queue = nullptr;

static void IRAM_ATTR switch_input_isr(void *arg)
{
    uint32_t gpio_num = SWITCH_INPUT_GPIO;
    xQueueSendFromISR(switch_evt_queue, &gpio_num, nullptr);
}

static void switch_input_task(void *arg)
{
    while (true) {
        uint32_t io_num;
        if (xQueueReceive(switch_evt_queue, &io_num, portMAX_DELAY)) {
            // Debounce: wait, then drain any additional events that
            // queued during the debounce window. This collapses a noisy
            // mechanical edge into a single logical event.
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_MS));
            while (xQueueReceive(switch_evt_queue, &io_num, 0) == pdTRUE) {}

            // Toggle the Matter OnOff attribute. The attribute update
            // triggers the standard callback path which drives the relay via relay_set().
            uint16_t endpoint_id = relay_endpoint_id;
            uint32_t cluster_id = OnOff::Id;
            uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

            attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);
            esp_matter_attr_val_t val = esp_matter_invalid(nullptr);
            attribute::get_val(attribute, &val);
            val.val.b = !val.val.b;
            attribute::update(endpoint_id, cluster_id, attribute_id, &val);
            ESP_LOGI(TAG, "Switch input toggled relay");
        }
    }
}

esp_err_t switch_input_init(void)
{
    // Create the ISR-to-task communication queue and the task that
    // services it. The task must exist before the ISR can fire
    // (otherwise events queue up with no consumer), so we create them
    // in this order.
    switch_evt_queue = xQueueCreate(QUEUE_DEPTH, sizeof(uint32_t));
    xTaskCreate(switch_input_task, "switch_input", TASK_STACK_SIZE,
                nullptr, TASK_PRIORITY, nullptr);

    // Configure GPIO10 as edge-interrupt input. See HARDWARE NOTE above
    // for why both internal pulls are disabled.
    gpio_config_t switch_cfg = {
        .pin_bit_mask = (1ULL << SWITCH_INPUT_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&switch_cfg);

    // Install the GPIO ISR service. ESP_ERR_INVALID_STATE means it was
    // already installed by another module (the Matter stack or
    // iot_button via button.cpp), which is fine — we share a single
    // ISR service across the whole app.
    esp_err_t isr_err = gpio_install_isr_service(0);
    if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to install ISR service: %d", isr_err);
    }

    esp_err_t handler_err = gpio_isr_handler_add(SWITCH_INPUT_GPIO,
                                                  switch_input_isr, nullptr);
    if (handler_err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add switch ISR handler: %d", handler_err);
    }

    ESP_LOGI(TAG, "Switch input initialized on GPIO%d", SWITCH_INPUT_GPIO);
    return ESP_OK;
}