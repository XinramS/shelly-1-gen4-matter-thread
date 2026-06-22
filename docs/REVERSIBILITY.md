# Warranty, Factory Keys, and Reversibility

**[README](../README.md)** > **Warranty, Factory Keys, and Reversibility** · [Report an issue](../../../issues/new)

This page explains what installing third-party firmware does to a Shelly 1 Gen4, whether you can return the device to stock, and what that depends on.

> ⚠️ **Default assumption: treat flashing as one-way.** You can only restore the device to factory state if you captured a full-chip backup over UART before flashing. Without that backup, flashing is irreversible. This is verified for the UART path and explained below.

---

## Contents

- [What flashing changes](#what-flashing-changes)
- [Factory keys](#factory-keys)
- [Can you go back](#can-you-go-back)
- [UART path, verified recoverable](#uart-path-verified-recoverable)
- [Web UI and OTA flash path, not yet investigated](#web-ui-and-ota-flash-path-not-yet-investigated)
- [Verification log](#verification-log)

---

## What flashing changes

Installing this firmware replaces the stock Shelly firmware, and that has consequences beyond the new features. It voids your warranty, and Shelly cannot provide technical support for a device running third-party code. It also removes the factory keys. Stock firmware uses factory-provisioned keys to reach Shelly Cloud and to receive official over-the-air (OTA) updates, so once the stock firmware is overwritten those services are unavailable until the original firmware is restored.

None of this is a criticism of Shelly. The device is deliberately flashable, which is what makes this project possible. The point of this page is to be honest about the trade-off so you can make an informed decision.

---

## Factory keys

Shelly devices are provisioned at the factory with per-device keys and certificates. These authenticate the device to Shelly Cloud and to the official OTA update service, and they are unique per device.

The keys reside in flash, in a factory or NVS type partition. A full-chip backup captures them regardless of which partition they sit in, because it copies every byte of the 8MB flash. Anything bound to eFuse, the one-time-programmable silicon, cannot be erased by flashing and survives no matter what you do. The recoverable-or-not question is therefore entirely about the flash-resident keys, and therefore entirely about whether your backup captured them. Restore the full flash image and the keys come back. Lose the image and they are gone.

---

## Can you go back

Reversibility depends on two things, not one. The first is the flash method, meaning UART, which can read the chip before writing, versus the Web UI or OTA path, which is write-only and cannot read first. The second is backup completeness, meaning a full 8MB image versus a partial image versus none at all.

| Method | Backup possible | Full-chip backup captures keys | Outcome |
|---|---|---|---|
| UART | Yes, reads the chip before writing | Yes | Recoverable |
| Web UI or OTA | No, write-only | No image exists | One-way |

This is not purely a matter of UART being safe and OTA being unsafe. A UART user who takes only a partial backup, or who runs `erase_flash` without a complete image to restore, is in the same one-way situation as an OTA user. Completeness, not just method, is what protects you. The rule that actually keeps you safe is to capture a full 8MB backup before flashing, or to accept that the change may be permanent.

---

## UART path, verified recoverable

The flashing procedure in the [Flashing Guide](FLASHING.md) reads the entire 8MB flash to a file before writing anything. Because that image includes the factory keys, restoring it returns the device to a genuine factory state.

This has been tested, not assumed. On a Shelly 1 Gen4 of hardware revision v0.1.2, a full 8MB backup was taken before flashing, then restored as a full image. After the restore the device connected to Shelly Cloud and successfully applied a cloud-delivered OTA update, moving from `1.7.0-gbe7545d` to `1.7.5-g9979d16`, on 2026-06-18. Both the Shelly Cloud connection and the cloud OTA update require the factory keys, so a successful round-trip confirms the keys were preserved in the backup and restored intact.

For the exact backup and restore steps, see [Restoring stock firmware](FLASHING.md#restoring-stock-firmware).

> **Scope.** This result is for one device on hardware revision v0.1.2, with a backup taken before the first flash. It is strong evidence that the UART full-chip method is reversible. It does not generalize to partial backups, other methods, or the OTA flash path below.

---

## Web UI and OTA flash path, not yet investigated

> ⚠️ **Status: not tested by this project.** This section is background, not instructions. This project does not yet ship a firmware image for this path.

Shelly's Web UI can install third-party firmware by uploading a file, without a UART connection. This is how projects such as ESPHome and Tasmota are commonly installed on Shelly hardware. This project has not investigated or confirmed that path on the Gen4.

One thing about this path is certain without testing. It is one-way. A Web UI flash cannot read the chip before writing, so no full-chip backup can be made, so the factory keys cannot be saved. A device flashed this way cannot be returned to a fully functional stock state.

The mechanics remain unknown. It is not confirmed whether the Gen4 Web UI accepts third-party firmware in practice, what image format it expects, which endpoint or RPC call handles the upload, which partition it writes to, or how that interacts with this firmware's layout. The image is likely a packaged artifact rather than a raw esptool `.bin`, and the ESPHome and Tasmota Shelly builds are the reference to study. Until this path is tested and recorded in the verification log, treat it as unsupported.

---

## Verification log

A running record of what has been tested, scoped to the exact conditions.

- **2026-06-18.** UART full-chip backup and restore on hardware revision v0.1.2. After restore, Shelly Cloud connected and a cloud OTA update applied, moving from `1.7.0-gbe7545d` to `1.7.5-g9979d16`. Confirms the UART full-chip path is reversible with the keys intact.

---

## Related documentation

- [README](../README.md) — project overview and quick start
- [Why Matter over Thread](WHY.md) — the rationale for Matter over Thread
- [Flashing Guide](FLASHING.md) — wiring, backing up stock firmware, and flashing
- [Commissioning](COMMISSIONING.md) — pairing the device and reading the status LED
- [Power Consumption](POWER.md) — measured draw and the Thread router design choice
- [Building from Source](BUILDING.md) — compiling the firmware yourself
- [Certification](CERTIFICATION.md) — uncertified status and test credentials
- [Roadmap](ROADMAP.md) — current known limitations and planned work
- [Contributing](CONTRIBUTING.md) — reporting bugs and the firmware filename convention
- [Contributors](CONTRIBUTORS.md) — people who have helped move the project forward
