#pragma once

#include <esp_err.h>

// Initialize the wall switch input (GPIO10 on Shelly 1 Gen4).
//
// Configures GPIO10 as an interrupt-driven input. On any edge (rising
// or falling), debounces for 50ms and then toggles the Matter OnOff
// attribute on the light endpoint.
//
// This implements the "Toggle" behavior expected when wiring a standard
// wall switch to the SW terminal — every flip of the switch toggles the
// relay state, regardless of switch position.
//
// HARDWARE NOTE: The Shelly 1 Gen4 PCB has an external pull resistor
// on GPIO10. Do not enable the internal pullup or pulldown — internal
// pulls will fight the external circuit and prevent the ISR from
// firing. See switch_input.cpp for details.
//
// Returns ESP_OK on success, error code on failure.
esp_err_t switch_input_init(void);