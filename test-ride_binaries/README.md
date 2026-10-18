# Precompiled Firmware Releases [version 2026-10-18] 🚀

This directory contains fully merged, production-ready factory images for **ESP32-MICRO-DOS**. 

Unlike standard firmware files, these `.factory.bin` images are monolithic. They **already include the bootloader, partition tables, system configs, and the core application code** stitched into a single file. You do not need any auxiliary files to flash your board.

## 📦 Supported Hardware Variants

Please select the specific binary that matches your exact display hardware:

### 🖥️ Resistive Display Architecture (Classic SPI)
*   **`ESP32-2432S028R.firmware.factory.bin`**
    *   Target: ESP32-WROOM **ESP32 Cheap Yellow Display (CYD)** 2.8" Resistive Touch Screen.
    *   Display Controller: ST7789 (SPI).
    *   Backlight pin 21
*   **`E32R40T.firmware.factory.bin`**
    *   Target: ESP32-DOWD 4.0" variant boards utilizing resistive touch mapping.
    *   Display Controller: ST7796 (SPI).
    *   Backlight pin 27
*   **`E32R32P.firmware.factory.bin`**
    *   Target: ESP32-DOWD 3.2" variant boards utilizing resistive touch mapping.
    *   Display Controller: ST7789 (SPI).
    *   Backlight pin 27

### 📱 Capacitive Display Architecture (High-Res QSPI)
*   **`JC3248W535C.firmware.factory.bin`**
    *   Target: ESP32-S3 3.5" Smart Display module.
    *   Display Controller: AXS15231B interface running over an I2C capacitive multi-touch grid.
    *   Backlight pin 1 

---

## ⚡ Flashing Instructions

> ⚠️ **CRITICAL REQUIREMENT:** Because these are factory images, they must be flashed starting exactly at offset address **`0x0`**.

### Method 1: Command Line (Recommended)
If you have `esptool.py` installed via Python or PlatformIO, connect your board via USB and execute the following command (replace with your specific filename):

```bash
esptool.py write_flash 0x0 ESP32-2432S028R.firmware.factory.bin
```

### Method 2: Graphical User Interface (GUI)
If using the official **Espressif Flash Download Tool**:
1. Add your chosen `.factory.bin` file to the top row.
2. Set the target destination address box to **`0x0`**.
3. Ensure no other rows are checked or active.
4. Click **Start**.

---

## 💾 Post-Flash Setup
After a successful flash, ensure you have copied example apps from SDCARD/ directory to **Micro SD card formatted to FAT32**. Micro SD card can be inserted (or ejected) at any time while operating. For CHAT.BIN, CODE.BIN and GO.BIN to be usable, there should be two files in SDCARD root directory, /WIFI.CFG and /OLLAMA.CFG.
