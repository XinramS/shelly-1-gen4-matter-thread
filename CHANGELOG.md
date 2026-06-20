# Changelog

All notable changes to this firmware are recorded here. The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project follows [semantic versioning](https://semver.org/spec/v2.0.0.html).

Each firmware variant versions independently, so releases are grouped by variant. Within the v1.x line of a variant the NVS schema is stable, and an in-place upgrade through the ESP-IDF CLI preserves commissioning. Switching between variants always requires erase and re-commission, because each variant is a different Matter device type.

## Light

### [1.2.1] - 2026-05-23

Module refactor, safety contract, and earlier init. No user-visible behavior change.

#### Added
- Documented relay safety contract. All relay control flows through a single `relay_set()` function with an auditable invariant: pre-init safe-low, single-point thermal lockout, Thread-loss hold, and panic recovery.

#### Changed
- Refactored the codebase into per-concern modules (`button`, `thermal`, `switch_input`, `relay`) alongside `status_led`, each logging under its own tag.
- Moved subsystem initialization before `esp_matter::start()`, so the relay reaches its safe-low state earlier in boot and thermal protection is active before the Matter stack starts.

### [1.2.0] - 2026-05-14

Identify cluster and Multi-Admin LED fix.

#### Added
- Matter Identify cluster. The status LED blinks 100ms on and 900ms off for the requested duration, then returns to its previous state.
- Hardware version reporting of `v0.1.2`, matching the PCB silkscreen, in place of the SDK test default.

#### Fixed
- Multi-Admin LED state. The LED no longer drops into the BLE advertising blink when a commissioning window opens on an already-commissioned device. It now stays solid through Multi-Admin commissioning.

#### Changed
- Pinned `CONFIG_OPENTHREAD_FTD` explicitly rather than relying on an inherited value.

### [1.1.0] - 2026-05-11

Status LED support.

#### Added
- Status LED indication with four patterns: off while starting up, rapid blink while BLE advertising, slow blink while joining Thread, and solid when connected.

#### Changed
- Release tags now include model, generation, and variant (`shelly-1-gen4-light-vX.Y.Z`) to support future variants in the same repository.

#### Notes
- The status LED is on GPIO15 with active-low polarity, determined by GPIO sweep testing on real hardware, not GPIO0 as ESPHome documentation states.

### [1.0.0] - 2026-05-09

First public release. Matter On/Off Light device type with latching relay behavior.

#### Added
- Matter over Thread. The device joins your Thread mesh and is controlled directly over Matter.
- Multi-fabric commissioning to Apple Home, Home Assistant, and Google Home at the same time.
- Physical button toggle on the onboard relay button.
- External wall switch input on the SW terminal, hardware-debounced.
- State sync across all controllers.
- Thread Router mode to extend your Thread mesh for other devices.
- Factory reset via long press of the onboard button.
- Verified stock firmware backup and restore.

## Opener

### [1.0.0] - 2026-06-16

First release of the Opener variant. Matter On/Off Plug-in Unit plus a Contact Sensor, built on the Light v1.2.1 module foundation and relay safety contract.

#### Added
- Momentary pulse relay. Turning the device on energizes the relay for a fixed pulse, then firmware returns it to off. The pulse is guaranteed in firmware through `relay_set()`, so a dropped command or a crashed hub cannot leave the relay energized. Default pulse width is 500ms.
- Door contact sensor on the SW terminal (GPIO10), exposed as a Matter Contact Sensor and reported to every commissioned fabric. In this variant the SW terminal is a sensor input, not a wall toggle.
- Three-endpoint Matter composition: an On/Off Plug-in Unit for the relay, a Contact Sensor for door state, and the Root Node.
- Compile-time constants in `main/app_priv.h`: `OPENER_PULSE_MS` (default 500) for pulse width and `CONTACT_OPEN_IS_HIGH` (default 0) for reed polarity.

## Outlet

### [1.0.0] - 2026-06-19

First release of the Outlet variant. Matter On/Off Plug-in Unit with latching relay behavior, built on the Light v1.2.1 module foundation and relay safety contract.

#### Added
- Latching relay presented as a Matter On/Off Plug-in Unit, so smart home apps display and voice-control the device as an outlet rather than a light. Electrically identical to the Light variant.
- External wall switch input on the SW terminal (GPIO10), hardware-debounced, toggling the relay as on the Light variant.
- Two-endpoint Matter composition: an On/Off Plug-in Unit for the relay, and the Root Node.

---

Full release notes, including tested scenarios, are on the [GitHub releases page](../../releases).
