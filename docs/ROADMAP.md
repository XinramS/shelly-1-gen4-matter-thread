# Roadmap

**[README](../README.md)** > **Roadmap** · [Report an issue](../../../issues/new)

This document tracks the current known limitations and the planned work. To prioritize something, vote on the [roadmap discussion](../../../discussions) or open an issue with your use case.

---

## Contents

- [Known limitations](#known-limitations)
- [Planned](#planned)

---

## Known limitations

- Matter OTA updates are supported and documented in Home Assistant only. Because the firmware uses test credentials, the assumption is that no other major Matter ecosystem will serve OTA updates to it. Commissioning-preserving updates run through Home Assistant's Matter Server. See [Updating](UPDATING.md).
- Thermal protection is implemented but not validated. The firmware reads ESP32-C6 die temperature and cuts the relay above 75°C, but the threshold has not been validated under controlled thermal conditions.

---

## Planned

- Shelly 1 Mini support.
- Shelly 2 PM support.
- A Sleepy End Device variant for battery and solar constrained installs, trading mesh extension for lower power.
- Build ergonomics. Migrate the build to consume esp-matter through the component library instead of a separately installed SDK, and fold the C6 and Thread settings into the base sdkconfig.defaults, so a plain `idf.py build` works without extra setup.
- Shared components directory across all variant builds.

---

## Related documentation

- [README](../README.md) — project overview and quick start
- [Why Matter over Thread](WHY.md) — the rationale for Matter over Thread
- [Flashing Guide](FLASHING.md) — wiring, backing up stock firmware, and flashing
- [Reversibility](REVERSIBILITY.md) — warranty, factory keys, and how reversible flashing is
- [Commissioning](COMMISSIONING.md) — pairing the device and reading the status LED
- [Updating](UPDATING.md) — keeping a device current after flashing
- [Power Consumption](POWER.md) — measured draw and the Thread Router design choice
- [Building from Source](BUILDING.md) — compiling the firmware yourself
- [Certification](CERTIFICATION.md) — uncertified status and test credentials
- [Contributing](CONTRIBUTING.md) — reporting bugs and the firmware filename convention
- [Contributors](CONTRIBUTORS.md) — people who have helped move the project forward
