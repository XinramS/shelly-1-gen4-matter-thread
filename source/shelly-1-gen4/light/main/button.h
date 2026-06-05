#pragma once

#include <esp_err.h>

typedef void *shelly_button_handle_t;

// Initialize the device button (GPIO4 on Shelly 1 Gen4).
//
// Configures the onboard button as an active-low input. On press-down,
// toggles the OnOff attribute on the light endpoint.
//
// The returned handle can be passed to app_reset_button_register() to
// also wire factory-reset behavior to the same physical button.
//
// Returns a button handle on success, NULL on failure.
shelly_button_handle_t button_init(void);