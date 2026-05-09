# Shelly 1 Gen 4 — Matter over Thread

[![License: Apache 2.0](https://img.shields.io/badge/License-Apache_2.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![Release](https://img.shields.io/github/v/release/automatous-io/shelly-1-gen4-matter-thread)](../../releases/latest)
[![Downloads](https://img.shields.io/github/downloads/automatous-io/shelly-1-gen4-matter-thread/total)](../../releases)
[![Stars](https://img.shields.io/github/stars/automatous-io/shelly-1-gen4-matter-thread?style=social)](../../stargazers)

> **⚠️ Disclaimer.** Flashing third-party firmware modifies your device and may void your warranty. Incorrect flashing can brick your device. Always back up your original firmware before proceeding. You assume all responsibility for any damage, data loss, or device failure. This project is not affiliated with Shelly, Allterco Robotics, CSA, or Espressif Systems.

Open source Matter over Thread firmware for the Shelly 1 Gen 4. Works natively with Apple Home, Google Home, Alexa, and Home Assistant — no WiFi, no cloud, no Shelly app, no subscription. **The Gen 4 ships with a thread radio - why not flash firmware that unlocks and uses it?**

<p align="center">
  <img src="docs/images/apple-home-device-info-v1.0.0.png" alt="Shelly 1 Gen 4 running this firmware in Apple Home, showing manufacturer, model, and firmware version" width="300">
</p>

*Shelly 1 Gen 4 running v1.0.0 of this firmware, commissioned to Apple Home. Manufacturer: AUTOMATOUS.IO, Model: Shelly 1 Gen4 MoT Light.*

> This release is the **Light variant** — Matter `On/Off Light` device type with **latching relay behavior**. The relay holds whatever state you set (on or off) until you change it again. The Shelly appears as a light bulb icon in your smart home app, and the relay can switch any "set and hold" load.
>
> **Power-on behavior:** Defaults to **off** at power-up. A configurable power-on state (off, on, restore last state) using the Matter `StartUpOnOff` attribute is planned for a future release. In the meantime, use a smart home automation as a workaround (e.g., HA: "when this device comes online, turn it on").
>
> A future **Switch variant** will add **momentary relay pulse** behavior for applications that expect a brief contact closure — garage door openers, doorbells, gates, and similar pulse-activated devices. Variants are separate firmware builds because Matter device types are declared at compile time and cannot be changed in the smart home app after commissioning.

> ⚠️ Uses ESP-Matter SDK test credentials (not VID/PID-certified). Functional for personal use; not suitable for resale as a certified Matter product.

---

## Contents

- [Quick Start](#quick-start)
- [Features](#features)
- [Compatible Hubs](#compatible-hubs)
- [Flashing](#flashing)
- [Commissioning](#commissioning)
- [Factory Reset](#factory-reset)
- [Known Issues / Roadmap](#known-issues--roadmap)
- [Building from Source](#building-from-source)
- [Matter and Thread Certification](#matter-and-thread-certification)
- [Contributing](#contributing)
- [Why?](#why)

---

## Quick Start
 
**Have a [Thread Border Router](#compatible-hubs), USB-UART adapter and 1.27mm to 2.54mm adapter? Flash and go.**

1. [Download the latest release](../../releases/latest) — grab the `automatous-io-shelly-1-gen4-light-vX.Y.Z.bin` file from the assets.
2. [Enter flash mode](#enter-flash-mode) on your Shelly.
3. [Back up your original firmware](#2-back-up-the-original-shelly-firmware) and flash the latest release with [ESPConnect](#flash-with-espconnect).
4. [Commission](#commissioning) with your smart home app.

---

## Features
 
- ✅ Matter over Thread — no WiFi required
- ✅ Compatible with Apple Home, Google Home, Alexa, Home Assistant
- ✅ Multi-fabric — commission to multiple hubs simultaneously
- ✅ Physical button toggle (onboard relay button)
- ✅ External wall switch input (SW terminal)
- ✅ State sync across all controllers
- ✅ Thread Router mode — extends your Thread mesh network for other devices
- ✅ Factory reset via long press (onboard relay button)

> **Note:** This firmware is best suited for lights, fans, outlets, heaters, and other "set and hold" loads. It is **not** suitable for garage door openers, doorbells, gates, or any device that expects a brief contact closure — see the [variant note](#shelly-1-gen-4--matter-over-thread) at the top for details on the future Switch variant.

---

## Compatible Hubs

> **Requires a Thread Border Router.** A Thread Border Router is required for any Matter-over-Thread device. See the table below for tested and supported options.

| Hub | Thread Border Router | Tested |
|-----|---------------------|--------|
| Apple HomePod mini | ✅ | ✅ |
| Apple HomePod (2nd gen) | ✅ | ✅ |
| Apple TV 4K (3rd gen) | ✅ | Untested |
| iPhone 15 Pro + | ✅ | ✅ |
| Google Nest Hub (2nd gen) | ✅ | Untested |
| Home Assistant (SkyConnect / Yellow) | ✅ | ✅ |
| Amazon Echo (4th gen) | ✅ | Untested |
 
---

## Flashing
 
### What you will need
- Shelly 1 Gen 4
- CP2102 USB-UART adapter
- 1.27mm 7-pin to 2.54mm Dupont custom cable or adapter board [(see pinout)](#cp2102-to-shelly-wiring)
- Chrome, Edge, or any Chromium-based browser (for Web Serial) or esptool via CLI
- (Optional) A separate serial monitor for viewing boot logs after flashing — see [Verify the firmware is running](#5-verify-the-firmware-is-running)

> Solid core Cat 5e / 6 ethernet wires should fit into the female square holes on the Shelly if you don't want to custom make an adapter or solder to flash - although much harder and finicky to manage. I purchased a small adapter board and soldered both 1.27mm and 2.54mm pins to it and utilized Dupont wires, as pictured below.

#### CP2102 to Shelly Wiring

> Full pinout and hardware overview: [Shelly 1 Gen 4 Knowledge Base](https://kb.shelly.cloud/knowledge-base/shelly-1-gen4-anz)

| CP2102 | Shelly 1 Gen 4 |
|--------|---------------|
| N/A | Pin 1 (ESP_DBG_UART) |
| RXD | Pin 2 (TXD) |
| TXD | Pin 3 (RXD) |
| 3.3V | Pin 4 (3.3V)|
| N/A | Pin 5 (RESET) |
| N/A | Pin 6 (GPIO0 - BOOT)|
| GND | Pin 7 (GND) |

> The crossover is built into the table above: connect CP2102 RXD to Shelly TXD, and CP2102 TXD to Shelly RXD. If you're wiring this from the CP2102's "TX" and "RX" labels directly to the Shelly's "TX" and "RX" labels without crossing, it will not work.

> **Do not connect the 5V pin.** The Shelly programming header is 3.3V only — applying 5V will likely permanently damage the ESP32-C6 and brick the device. Many CP2102 boards have both 3.3V and 5V pins; verify you are using the 3.3V pin before connecting.

![Shelly 1 Gen 4 with Automatous flashing adapter and CP2102 USB-UART](docs/images/shelly1gen4-adapter-and-cp2102.jpg)

*The Shelly 1 Gen 4 with the flashing adapter mounted, alongside a CP2102 USB-UART adapter.*

![Adapter mounted on Shelly programming header](docs/images/shelly1gen4-adapter-mounted.jpg)

*The 1.27mm-to-2.54mm adapter board seated next to the Shelly's 7-pin programming header.*

![Complete flashing setup with all wires connected](docs/images/shelly1gen4-flash-setup-wired.jpg)

*Full flashing setup: Shelly's programming header connected to the CP2102 via Dupont wires, with the GPIO0-to-GND jumper visible (white wire) for entering flash mode.*

#### Enter Flash Mode

> ## ⚠️ Safety Warning
>
> **Never connect the Shelly to AC mains while flashing.** The programming header is not galvanically isolated from the relay circuitry. Connecting the Shelly to mains voltage AND a USB-UART adapter simultaneously can cause:
> - Personal electrocution risk
> - Permanent destruction of the Shelly
> - Permanent destruction of your computer's USB port (or the entire computer)
>
> Always flash the Shelly powered only by the 3.3V line from the USB-UART adapter, with no other connections to the device. Do not flash a Shelly that is installed in a wall, junction box, or any AC-connected fixture. Remove the Shelly entirely from any AC wiring before flashing.

The Shelly enters bootloader (flash) mode when GPIO0 is held low at power-up.

1. **Disconnect** the 3.3V line if currently connected.
2. **Bridge** Pin 6 (GPIO0) to Pin 7 (GND) — keep them connected.
3. **Connect** the 3.3V line to power the device. The Shelly boots into flash mode.
4. You can now leave the GPIO0–GND bridge in place for the duration of flashing, or remove it after a second or two — either works once the bootloader is running.

### Flash with ESPConnect

ESPConnect is a browser-based ESP32 flashing tool built on Web Serial. No installation required — works in Chrome, Edge, or any Chromium-based browser on macOS, Windows, and Linux.

#### 1. Connect to the Shelly

1. Put the Shelly into flash mode (Pin 6 GPIO0 bridged to Pin 7 GND, then power up via 3.3V).
2. Open [ESPConnect](https://thelastoutpostworkshop.github.io/microcontroller_devkit/espconnect/) in your browser.
3. Click **Connect** and select your USB-UART serial port from the browser dialog.
4. Set baud rate to **115200**. Slower than the default, but more reliable for the long backup read.
5. ESPConnect will display chip info confirming the ESP32-C6 is detected, including the device's **MAC address** — note this down, you'll use it to label your backup file. If you see "Failed to connect" or no chip info, the Shelly is not in flash mode — recheck the GPIO0–GND bridge and re-power the device.

#### 2. Back up the original Shelly firmware

> This step is essential. The backup is the only way to restore your device to its factory state. **Do not skip it**.

1. In the left navigation, click **Flash Tools**.
2. Click **Download Flash Backup**.
3. ESPConnect will read the entire 8MB flash and download it as a `.bin` file. This takes 5–10 minutes at 115200 baud — do not disconnect or close the browser during the read.
4. Rename the downloaded file to include the device's MAC address, e.g. `shelly-1-gen4-stock-AABBCCDDEEFF.bin`.
5. **Verify the file size is approximately 8 MB (8,388,608 bytes).** A smaller file means the read was incomplete or interrupted; redo the backup before proceeding.
6. Store the backup somewhere safe. If you are flashing multiple Shellies, keep each MAC-labeled backup separate — they should not be interchanged.

#### 3. Flash the Automatous firmware

1. Still in **Flash Tools**, click **Flash Firmware**.
2. Select the latest release `.bin` from your downloads.
3. Set flash offset to `0x0`.
4. Check **Erase entire flash before writing**.
5. Click **Flash** and wait for the operation to complete (1–2 minutes). ESPConnect shows a progress bar and a success message when done.

#### 4. Boot the new firmware

1. Disconnect from ESPConnect (click **Disconnect** or close the tab).
2. **Remove the GPIO0 ↔ GND bridge.** This is critical — if GPIO0 is still bridged to GND at the next power-up, the Shelly will boot back into flash mode instead of running your firmware.
3. Power-cycle the Shelly: disconnect the 3.3V line, wait two seconds, reconnect.
4. The Shelly is now running the Automatous firmware and is in BLE commissioning mode, ready to be added to your smart home ecosystem.

#### 5. Verify the firmware is running

After power-cycling, the Shelly is running your firmware and advertising for Matter commissioning over BLE. You can verify it's working in either of two ways:

**Easiest: try to add it from your smart home app.** Open Apple Home, Google Home, Alexa, or Home Assistant and start the "add device" flow. If the Shelly appears as a discoverable Matter device, the firmware is running correctly. Proceed to [Commissioning](#commissioning).

**For developers / troubleshooting:** ESPConnect requires flash mode and cannot show normal application serial output easily from my experience with this project. Use a separate serial monitor instead:

- **macOS / Linux CLI:** `screen /dev/cu.usbserial-XXXX 115200` (replace with your actual port)
- **macOS GUI:** [Serial](https://serial.app/) or [CoolTerm](https://freeware.the-meiers.org/)
- **Windows:** [Tera Term](https://teratermproject.github.io/) or PuTTY at 115200 baud
- **VS Code:** the Serial Monitor extension

You should see boot logs followed by `Commissioning Window Opened`. This confirms Matter is initialized and the device is advertising.

> ⚠️ **Important:** With your firmware running, do not bridge GPIO0 to GND again unless you want to re-enter flash mode. The USB-UART 3.3V line can stay connected for power and serial monitoring during commissioning — just leave Pin 6 (GPIO0) disconnected.

#### Restoring stock firmware

> ✅ **Stock restore is verified working** — your Shelly will create its `shelly-XXXXXX` setup AP, pair with the Shelly app, and behave identically to a factory unit.

If you want to revert to the original Shelly firmware:

1. Put the Shelly back into flash mode (GPIO0 ↔ GND, power up via 3.3V).
2. In ESPConnect, click **Flash Firmware**.
3. Select the `.bin` backup file you saved earlier (matching this device's MAC address).
4. Set flash offset to `0x0`, check **Erase entire flash before writing**, click **Flash**.
5. Power-cycle the Shelly. It will boot the restored firmware and behave as factory.

<details>
<summary>Command line flashing with esptool (advanced)</summary>

Requires [esptool](https://github.com/espressif/esptool) installed. On macOS/Linux: `pip install esptool`. On Windows: install via pip, or download the standalone binary from the releases page.

Throughout these commands, replace:
- `<PORT>` with your serial port (`/dev/cu.usbserial-XXXX` on macOS, `/dev/ttyUSB0` on Linux, `COM3` on Windows)
- `<MAC>` with the Shelly's MAC address (read from `chip_id` below)
- `<VERSION>` with the firmware release version (e.g. `v1.0.0`)

Put the Shelly into flash mode before each command (GPIO0 bridged to GND, then power up via 3.3V).

**Identify the chip and read the MAC address:**

Device information that includes Mac Address:
```bash
esptool.py --chip esp32c6 --port <PORT> --baud 115200 chip_id
```

Backup your original firmware:

```bash
esptool.py --chip esp32c6 --port <PORT> --baud 115200 \
  read_flash 0x0 0x800000 shelly-1-gen4-stock-<MAC>.bin
```
This reads the entire 8 MB flash. Verify the resulting file is exactly 8,388,608 bytes before proceeding.

Erase flash:
```bash
esptool.py --chip esp32c6 --port <PORT> --baud 115200 erase_flash
```
 
**Flash the Automatous firmware:**

```bash
esptool.py --chip esp32c6 --port <PORT> --baud 115200 \
  write_flash 0x0 automatous-io-shelly-1-gen4-light-<VERSION>.bin
```
 
**Restore the original firmware:**

```bash
esptool.py --chip esp32c6 --port <PORT> --baud 115200 \
  write_flash 0x0 shelly-1-gen4-stock-<MAC>.bin
```

After flashing, remove the GPIO0–GND bridge and power-cycle the Shelly to boot the new firmware.
 
</details>

---

## Commissioning

- Commission devices ONE AT A TIME.
- Power off or move away other un-commissioned devices while setting up each one.
- All devices share the same pairing code, so the controller may get confused if multiple are advertising simultaneously.
- Once commissioned, each device gets a unique identity on your network.
- Disconnect UART from the Shelly and install wherever you need to.
 
### Apple Home

Scan this QR code with the Home app, or enter the setup code manually:

![Matter Setup QR Code](docs/images/matter_qrcode_20202021_3840.png)

1. Open the **Home** app.
2. Tap **+** → **Add Accessory**.
3. Scan the QR code above, or tap **More Options** to enter the setup code manually.
4. Press **Add Anyway** when prompted (this is expected — see [Matter and Thread Certification](#matter-and-thread-certification)).
5. Name the device and place it in a room.

```
Setup code: 3497-011-2332
```

 >**Note for users coming from HomeKit Bridge / Home Assistant setups:** HomeKit's "reclassify as Fan / Light / Switch" option (available when bridging a stock Shelly through Home Assistant) is not available with native Matter devices. The Matter device type is declared by the firmware at flash time and cannot be changed in the Apple Home app. If you need a different device type, flash the corresponding [variant](#shelly-1-gen-4--matter-over-thread) when one is available.


> ⚠️ This is the ESP-Matter SDK test setup code, used by all devices running this firmware. Once a device is commissioned to your Matter fabric, the setup code is no longer used for authentication — your Matter ecosystem manages credentials going forward. Multiple uncommissioned devices broadcasting the same setup code is why the commissioning instructions specify one device at a time.
 
<details>
<summary>Google Home / Alexa / Home Assistant commissioning</summary>
Google Home and Alexa support Matter commissioning via their respective apps. Use setup code `3497-011-2332` or the QR Code above when prompted. Google and Alexa hubs have not been directly tested but the firmware should work with any Matter-compliant hub. Let me know and we'll update compatible hubs section!
 
</details>

---

## Factory Reset
 
Hold the onboard relay button for **several seconds**. The device will reset and re-enter BLE commissioning mode. Make sure to remove the device from whatever ecosystems you've added it to.

---
 
## Known Issues / Roadmap
 
- [ ] Configurable power-on state — implement Matter `StartUpOnOff` attribute so users can choose default behavior (off, on, restore last state) after power loss. Currently defaults to off.
- [ ] Status LED — onboard LED indication for BLE advertising, Thread connecting, and Thread connected states is not currently functional.
- [ ] Switch variant — momentary relay pulse (~500ms) for garage door openers, doorbells, gates, and similar pulse-activated devices. Separate firmware build since Matter device types are declared at compile time.
- [ ] Thermal protection validation — the firmware reads ESP32-C6 die temperature and includes logic to cut off the relay above 75°C, but this has not been validated under controlled thermal conditions. Feature is implemented but not confirmed working as documented.

---

## Building from Source
 
<details>
<summary>Build instructions</summary>
 
Throughout these commands, replace:
- `<VERSION>` with the firmware release version (e.g. `v1.0.0`)

**Requirements:**
- ESP-IDF v5.5.2
- ESP-Matter (latest)
- macOS or Linux

Compile and build bin files for flash:
```bash
git clone https://github.com/automatous-io/shelly-1-gen4-matter-thread.git
cd shelly-1-gen4-matter-thread/source/light
# source from wherever you installed esp-idf and esp-matter. I installed in the locations below but yours could be different.
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh
idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults.c6_thread_shelly" set-target esp32c6
idf.py build
```

Merge bin files:
```bash
esptool.py --chip esp32c6 merge_bin \
  -o automatous-io-shelly-1-gen4-light-<VERSION>.bin \
  --flash_mode dio \
  --flash_size 8MB \
  0x0     build/bootloader/bootloader.bin \
  0x10000 build/partition_table/partition-table.bin \
  0x11000 build/ota_data_initial.bin \
  0x20000 build/light.bin
```
</details>

---

## License
 
Apache 2.0 — see [LICENSE](LICENSE)
 
Based on the [ESP-Matter](https://github.com/espressif/esp-matter) light example by Espressif Systems, licensed under Apache 2.0.

---

## Matter and Thread Certification

This firmware is **not** Matter Certified or Thread Certified. It implements the Matter protocol via Espressif's [esp-matter](https://github.com/espressif/esp-matter) SDK and Thread via OpenThread, but has not been submitted to the Connectivity Standards Alliance (CSA) or Thread Group for certification. Certification requires CSA membership ($7,000+/year) and Authorized Test Lab evaluation, which are out of scope for an open-source community project.

Devices flashed with this firmware will appear in Matter ecosystems as **uncertified Matter devices**. Most ecosystems show a one-time warning during commissioning ("Add Anyway" in Apple Home); this is normal and expected.

This firmware uses the publicly available ESP-Matter test VID/PID and setup code, which is acceptable for personal use and development but cannot be used in commercial certified products.

"Matter" is a trademark of the Connectivity Standards Alliance. "Thread" is a trademark of the Thread Group. "Shelly" is a trademark of Allterco Robotics. This project is not affiliated with, endorsed by, or sponsored by any of these organizations.

---

## Contributing

Contributions are welcome. This is a solo-maintained project, so response times will vary, but every issue and PR gets read.

### Reporting bugs

Open a [GitHub issue](../../issues/new) with:

- **Hardware:** Shelly 1 Gen 4 hardware revision (printed on the PCB), and the firmware version you're running
- **Ecosystem:** Apple Home, Google Home, Alexa, Home Assistant, etc., and the Thread Border Router(s) on your network
- **What happened:** What you expected, what actually occurred, and steps to reproduce
- **Logs:** Serial monitor output at 115200 baud, especially anything around the failure point

If your device won't commission, won't boot, or behaves unexpectedly after flashing, serial logs are usually the fastest path to a diagnosis.

---

## Why?

The Shelly 1 Gen 4 ships with an ESP32-C6 that includes an 802.15.4 Thread radio. Shelly's stock firmware uses only Matter over WiFi — the Thread radio sits unused. Shelly has stated they have no plans to add Thread support. This firmware unlocks it.

### Why Matter over Thread, and not ESPHome or Tasmota?

ESPHome and Tasmota are excellent firmware projects, but both effectively assume you have a central controller — Home Assistant for ESPHome, an MQTT broker plus a controller for Tasmota. They're great if that's your setup. They're a problem if it isn't.

This firmware speaks Matter directly. There's no middleware required, no HA instance, no MQTT broker, no ESPHome dashboard, no YAML configs, no bridge:

- **Apple Home user without Home Assistant?** It just works. Commission directly from the Home app.
- **Google Home or Alexa user?** Same — direct Matter commissioning, no third-party hub.
- **Home Assistant user?** Also works — commission via HA's Matter integration.
- **Multi-ecosystem household?** Multi-fabric commissioning lets the same device live in Apple Home, HA, and Google Home simultaneously.

Matter is an industry-standard protocol — your devices work with the ecosystems you have today, and remain compatible with new Matter ecosystems as they appear.

### Why Thread instead of WiFi?

Matter over Thread offers lower power consumption, lower latency, and mesh networking that doesn't depend on your WiFi. Devices on a Thread network can use any compatible Thread Border Router (iPhone 15 Pro+, HomePod mini, Apple TV 4K, Nest Hub 2nd gen, HA SkyConnect/Yellow) and continue to function even if your WiFi is down or congested.

I built this because I needed lower-power smart home devices in my Sprinter van where battery and solar capacity are limited and WiFi isn't always reliable. If you're an off-grid, low-power, or simply Thread-curious user, you might find this useful too.

---

## About

This firmware was created from inside an old Mercedes Sprinter T1N named Mabel, parked somewhere in the USA.

I put many sleepless nights obsessing about Thread support that kickstarted me into writing custom firmware and testing using ESP-IDF, ESP-Matter, and ESPHome examples — figuring out Shelly's custom partition offsets, GPIO quirks specific to this device, and getting Matter over Thread commissioning working on non-devkit hardware.

If it saved you the same headache, consider leaving a ⭐ on the repo — it helps the project show up in GitHub search and signals to other Shelly 1 Gen 4 owners that this firmware exists and works.