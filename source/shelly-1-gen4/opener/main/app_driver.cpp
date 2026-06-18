//
// Copyright 2026 AUTOMATOUS.IO
// Portions derived from Espressif esp-matter examples,
// originally released into the public domain / CC0.
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

// app_driver.cpp

#include <esp_log.h>
#include <esp_timer.h>

#include <esp_matter.h>
#include <app_priv.h>
#include <common_macros.h>

#include <relay.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

extern uint16_t relay_endpoint_id;

// Momentary pulse timer. On relay-on, arm a one-shot for OPENER_PULSE_MS,
// then turn the relay back off by clearing the OnOff attribute (not via
// relay_set, so the Matter-reported state returns to off too).
static esp_timer_handle_t s_pulse_timer = nullptr;

static void pulse_timer_cb(void *arg)
{
    esp_matter_attr_val_t val = esp_matter_bool(false);
    attribute::update(relay_endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id, &val);
}

static void pulse_timer_arm(void)
{
    if (s_pulse_timer == nullptr) {
        const esp_timer_create_args_t args = {
            .callback = pulse_timer_cb,
            .name = "opener_pulse",
        };
        if (esp_timer_create(&args, &s_pulse_timer) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to create pulse timer");
            return;
        }
    }

    // Restart cleanly if a previous pulse is still pending.
    esp_timer_stop(s_pulse_timer);
    esp_timer_start_once(s_pulse_timer, (uint64_t)OPENER_PULSE_MS * 1000);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    esp_err_t err = ESP_OK;
    if (endpoint_id == relay_endpoint_id) {
        if (cluster_id == OnOff::Id) {
            if (attribute_id == OnOff::Attributes::OnOff::Id) {
                err = relay_set(val->val.b);
                // Momentary: schedule auto-off after the pulse window.
                if (val->val.b) {
                    pulse_timer_arm();
                }
            }
        }
    }
    return err;
}

esp_err_t app_driver_relay_set_defaults(uint16_t endpoint_id)
{
    esp_err_t err = ESP_OK;
    esp_matter_attr_val_t val = esp_matter_invalid(NULL);

    attribute_t *attribute = attribute::get(endpoint_id, OnOff::Id, OnOff::Attributes::OnOff::Id);
    attribute::get_val(attribute, &val);
    err = relay_set(val.val.b);

    return err;
}