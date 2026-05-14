# Building from Source

[← Back to README](../README.md) · [Report an issue](../../../issues/new)

This guide covers building the Automatous firmware from source. If you just want to flash pre-built firmware, see the [Flashing Guide](FLASHING.md) instead — you don't need to build anything.

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

- **ESP-IDF v5.5.2** — Espressif's IoT development framework
- **ESP-Matter (latest)** — Espressif's Matter SDK
- **macOS or Linux** — Windows build path is not currently tested

Throughout these commands, replace:
- `<VERSION>` with the firmware release version (e.g. `v1.2.0`)

---

## Repository structure

The repository organizes firmware by product and variant:

```
source/
└── shelly-1-gen4/
    ├── light/           # Matter On/Off Light (latching relay)
    ├── light-switch/    # Matter On/Off Light Switch (planned, contributor expressed interest)
    └── switch/          # Matter On/Off with momentary pulse (planned)
```

Each variant is a self-contained ESP-IDF project. Build commands run from inside the variant directory. The examples below build the `light` variant; to build a different variant, substitute its directory in the `cd` command.

---

## Setup

If you don't already have ESP-IDF and ESP-Matter installed, follow Espressif's official setup guides first:

- [ESP-IDF Get Started](https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32c6/get-started/index.html)
- [ESP-Matter Get Started](https://docs.espressif.com/projects/esp-matter/en/latest/esp32c6/developing.html)

This project assumes you have both SDKs installed and configured for ESP32-C6 target.

---

## Build

Clone the repository and build the light variant:

```bash
git clone https://github.com/automatous-io/shelly-1-gen4-matter-thread.git
cd shelly-1-gen4-matter-thread/source/shelly-1-gen4/light

# source from wherever you installed esp-idf and esp-matter
# the locations below are examples — adjust to match your install paths
source ~/esp/esp-idf/export.sh
source ~/esp/esp-matter/export.sh

idf.py -DSDKCONFIG_DEFAULTS="sdkconfig.defaults.c6_thread_shelly" set-target esp32c6
idf.py build
```

The build produces individual `.bin` files in the `build/` directory.

---

## Merge bin files

To create a single combined `.bin` file suitable for flashing at offset `0x0` (matching the release binaries):

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

Once you have the merged `.bin` file, follow the [Flashing Guide](FLASHING.md) to write it to your Shelly. The flashing process is identical whether the firmware came from a release or your own build.

---

[← Back to README](../README.md)