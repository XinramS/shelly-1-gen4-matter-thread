# Roadmap

**[README](../README.md)** > **Roadmap** · [Report an issue](../../../issues/new)

This document tracks the current known limitations and the planned work. To prioritize something, vote on the [roadmap discussion](../../../discussions) or open an issue with your use case.

---

## Contents

- [Known limitations](#known-limitations)
- [Planned](#planned)

---

## Known limitations

- OTA updates are not yet supported. Updates currently require re-flashing over USB-UART. Flashing a merged binary through a web flasher also erases the NVS partition, so the device must be re-commissioned. An in-place upgrade that preserves commissioning is possible today only through the ESP-IDF CLI with `idf.py flash`.
- Thermal protection is implemented but not validated. The firmware reads ESP32-C6 die temperature and cuts the relay above 75°C, but the threshold has not been validated under controlled thermal conditions.

---

## Planned

- OTA updates. Exploratory, tracked for a future release.
- Upgrade in place for web flasher users. A flash manifest that writes the three component binaries (bootloader, partition table, and application) at their offsets without erasing the nvs partition, so Matter commissioning and Thread credentials survive a web-flash update. In progress.
- Shelly 1 Mini support. Planned, pending an easier flashing method, since the Mini exposes programming pads rather than a female header.
- A Sleepy End Device variant for battery and solar constrained installs, trading mesh extension for lower power.
- Build ergonomics. Migrate the build to consume esp-matter through the component library instead of a separately installed SDK, and fold the C6 and Thread settings into the base sdkconfig.defaults, so a plain `idf.py build` works without extra setup.

---

## Related documentation

- [README](../README.md) — project overview and quick start
- [Why Matter over Thread](WHY.md) — the rationale for Matter over Thread
- [Flashing Guide](FLASHING.md) — wiring, backing up stock firmware, and flashing
- [Reversibility](REVERSIBILITY.md) — warranty, factory keys, and how reversible flashing is
- [Commissioning](COMMISSIONING.md) — pairing the device and reading the status LED
- [Power Consumption](POWER.md) — measured draw and the Thread router design choice
- [Building from Source](BUILDING.md) — compiling the firmware yourself
- [Certification](CERTIFICATION.md) — uncertified status and test credentials
- [Contributing](CONTRIBUTING.md) — reporting bugs and the firmware filename convention
- [Contributors](CONTRIBUTORS.md) — people who have helped move the project forward
