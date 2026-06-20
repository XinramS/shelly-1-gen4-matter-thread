# Building from Source

**[README](../README.md)** > **Building from Source** · [Report an issue](../../../issues/new)

This guide covers building the Automatous firmware from source. If you just want to flash pre-built firmware, see the [Flashing Guide](FLASHING.md) instead. You do not need to build anything.

---

## Contents

- [Requirements](#requirements)
- [Repository structure](#repository-structure)
- [Setup](#setup)
- [Build](#build)
- [Merge bin files](#merge-bin-files)
- [Flash your build](#flash-your-build)

---

## Requirements

- **ESP-IDF v5.5.2**, Espressif's IoT development framework.
- **ESP-Matter (latest)**, Espressif's Matter SDK, installed as a cloned repository. The build reads it through the `ESP_MATTER_PATH` environment variable, which esp-matter's `export.sh` sets. It is not pulled from the component registry.
- **macOS or Linux**. The Windows build path is not currently tested.

---

## Repository structure

The repository organizes firmware by product and variant:

```
source/
└── shelly-1-gen4/
    ├── light/           # Matter On/Off Light, latching relay (released)
    ├── opener/          # Matter On/Off Plug-in Unit + Contact Sensor, momentary pulse (released)
    ├── outlet/          # Matter On/Off Plug-in Unit, latching relay, SW kept as a wall toggle (released)
    └── light-switch/    # Matter On/Off Light Switch, detached relay + SW input bound to other Matter devices (released)
```

Each variant is a self-contained ESP-IDF project. Build commands run from inside the variant directory. The examples below build the `light` variant; to build a different variant, substitute its directory in the `cd` command.

---

## Setup

If you don't already have ESP-IDF and ESP-Matter installed, follow Espressif's official setup guides first:

- [ESP-IDF Get Started](https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32c6/get-started/index.html)
- [ESP-Matter Get Started](https://docs.espressif.com/projects/esp-matter/en/latest/esp32c6/developing.html)

This project assumes you have both SDKs installed. The build step below sets the ESP32-C6 target.

---

## Build

Clone the repository and build the light variant:

```bash
git clone https://github.com/automatous-io/shelly-1-gen4-matter-thread.git
cd shelly-1-gen4-matter-thread/source/shelly-1-gen4/light

# source from wherever you installed esp-idf and esp-matter
# the locations below are examples, adjust to match your install paths
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh

idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults.c6_thread_shelly" set-target esp32c6
idf.py build
```

Run these in the same shell where you sourced the two `export.sh` scripts. ESP-IDF and the `ESP_MATTER_PATH` the build needs will not be set otherwise.

The `-DSDKCONFIG_DEFAULTS` flag points the build at `sdkconfig.defaults.c6_thread_shelly`, which holds the C6 and Thread settings. Without it the build will not produce a Matter over Thread image. `set-target` applies it once, and after that plain `idf.py build` rebuilds incrementally.

The build produces individual `.bin` files in the `build/` directory. The application image is named after the variant, so the light build produces `build/light.bin` and the opener build produces `build/opener.bin` and so on. The `Project build complete` line at the end of the build prints its exact path.

---

## Merge bin files

Replace `<VERSION>` with the release version, for example `v1.2.1`. The example merges the `light` build; for another variant, substitute its name in both the output filename and the `build/<variant>.bin` application image, for example `build/opener.bin`. To create a single combined `.bin` file suitable for flashing at offset `0x0`, matching the release binaries:

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

---

## Flash your build

There are two ways to flash a build.

**Merged binary, full install.** Flash the merged `.bin` with the [Flashing Guide](FLASHING.md), the same process used for the released binaries. This erases the whole flash, including the `nvs` partition, so a commissioned device loses its Matter fabric and must be re-commissioned. Use this for a first flash over stock firmware or any clean install.

**ESP-IDF CLI, preserves commissioning.** To update a device already running this firmware without re-pairing, put the Shelly into [flash mode](FLASHING.md#enter-flash-mode), connect the UART adapter, and from the variant directory run the following. Replace `<PORT>` with your USB-UART serial port.

```bash
idf.py -p <PORT> flash monitor
```

This writes the bootloader, partition table, and application image but leaves the `nvs` partition intact, so the Matter fabric and Thread credentials are preserved and the device does not need re-commissioning. No merge step is needed for this path. Do not use `idf.py erase-flash`, which wipes everything including commissioning.

---

## Related documentation

- [README](../README.md) — project overview and quick start
- [Why Matter over Thread](WHY.md) — the rationale for Matter over Thread
- [Flashing Guide](FLASHING.md) — wiring, backing up stock firmware, and flashing
- [Reversibility](REVERSIBILITY.md) — warranty, factory keys, and how reversible flashing is
- [Commissioning](COMMISSIONING.md) — pairing the device and reading the status LED
- [Power Consumption](POWER.md) — measured draw and the Thread router design choice
- [Certification](CERTIFICATION.md) — uncertified status and test credentials
- [Roadmap](ROADMAP.md) — current known limitations and planned work
- [Contributing](CONTRIBUTING.md) — reporting bugs and the firmware filename convention