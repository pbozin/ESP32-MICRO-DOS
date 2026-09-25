<img width="200" alt="shell" src="https://github.com/user-attachments/assets/8d52df68-ab0f-4e3b-826a-33f8e01c15d0" />
<img width="200" alt="bricks" src="https://github.com/user-attachments/assets/e8cf1f2c-280f-4051-b826-3669cdfcb3c0" />
<img width="200" alt="chat" src="https://github.com/user-attachments/assets/782b2ec2-74e2-42a8-af48-0b7fd8c68536" />
<img width="200" alt="chess" src="https://github.com/user-attachments/assets/3ef64f03-ed15-4048-ae7f-aa57d674d884" />

# ESP32 MicroDOS

An advanced, retro-style retro-operating system and interactive environment designed for 
CYD clones [https://www.lcdwiki.com/4.0inch_ESP32-32E_Display] and capacitive equivalents (JC3248). 

Unlike typical monolithic microcontroller firmware, **MicroDOS** behaves like a classic 1980s disk operating system with modern AI features. It features a custom multi-statement **BASIC interpreter**, a **Universal Dynamic Linker/Loader** capable of running precompiled native C binaries from an SD card, an indexed **4-bit Sprite Game Engine**, and a memory-efficient **Ollama LLM live-streaming engine** that can autonomously re-program the host system on the fly.

---

## 🌟 Core System Architecture

### 1. Dual Hardware Abstraction Layer (HAL)
MicroDOS supports two highly popular, low-cost smart display paradigms out of the box via compiler flags:
* **Classic CYD (`BOARD_CYD`)**: Targeting the E32N40T board variant utilizing SPI-based resistive touch and `TFT_eSPI` register calls.
* **Capacitive High-Res Display (`BOARD_JC3248`)**: Targeting QSPI smart-displays (such as the AXS15231B architecture) over `Arduino_GFX` bound to an I2C capacitive touch grid controller.

### 2. Universal Dual-Segment Relocation Loader (`EXEC`)
A feature typically reserved for full application-class processors. MicroDOS breaks the flat-memory constraint of microcontrollers by acting as an on-device **Dynamic Linker & Loader** for custom `.BIN` files compiled on a PC:
* **Phase 1 (Literal Pool Patches):** Scans instruction tracks block-by-word inside isolated IRAM staging spaces, capturing relative file offsets and updating them to absolute physical addresses allocated on the ESP32 instruction bus (`MALLOC_CAP_EXEC`).
* **Phase 2 (GOT Table Mapping):** Crawls the application's internal Global Offset Table (GOT) lookup grid to update indirect function pointers and global variables before spinning up a standalone, stack-guarded, runtime isolated FreeRTOS thread worker.

### 3. AI Stream Engine & Autonomous Code Harvesting
MicroDOS implements a lightweight token streaming state-machine that communicates with localized or remote **Ollama AI Servers**. 
* **JSON Stream Tunneling:** Instead of inflating the memory footprint with heavy JSON string parsers, it parses network socket buffers byte-by-byte, using a sliding memory footprint window (`memmove`) to roll old history lines outward once bounds thresholds fill up.
* **Code Injection Hook:** When contextually bound to `coder` mode, the terminal monitors incoming markdown tags. Encountering a ````basic```` markdown code block causes the OS to dump active interpretation tracks, capture the coming AI stream verbatim, and automatically inject the code into memory line-by-line via `storeLine()`. **The AI can rewrite the terminal program while it runs.**

### 4. 4-Bit Color Sprite & Game Matrix
To maintain fluid, zero-flicker rendering loops under rigorous RAM constraints, MicroDOS includes a specialized graphics compositor:
* **Indexed Color Compression:** Uses a 16-color global fixed-index palette matrix (`ramOSPalette`), shrinking active image requirements down to 4 bits per pixel.
* **Dirty-Rect Eraser Engine:** Sprites track their historical coordinate intersections (`isOnScreen`). Rather than wiping the display buffer on ticks, the system automatically uses hardware-accelerated RLE decompression to restore *only* the specific background slices altered by moving graphics before blitting new structures.
* **Chroma-Key Transparency:** Absolute color block Index 15 (`TFT_DARKGREY`) behaves as a mask bypass. The blitter maps coming pixel indices sequentially, dropping operations on Index 15 coordinates to allow background transparency.

---

## ⌨️ Built-in Shell Utility Dictionary

Run these commands directly inside the interactive interactive touch console interface:

| Command | Classification | Functional Profile Description |
| :--- | :--- | :--- |
| `HELP` | System | Displays the system command vocabulary matrix grouped by utility type. |
| `FREE` | System | Queries the Espressif system API heap space counters to dump live available memory bytes. |
| `DIR` / `CAT` | Storage | Scans the file directory tree on the micro SD card, rendering names and explicit byte allocations. |
| `EDIT <file>` | Storage | Launches a modal line-by-line interactive text utility allowing engineers to build, list, append, or patch scripts on the fly. |
| `LIST [r1, r2]`| Interpreter | Outputs lines matching target script ranges stored inside the interpretation buffer stack. |
| `RUN [file]` | Interpreter | Dispatches the text interpreter engine, executing local or SD-stored `.BAS` files line by line. |
| `EXEC <binary>`| Native | Commands the dynamic linker to resolve, relocate, inject, and launch native precompiled compiled applications. |
| `DUMP / DUMPS` | Forensic | Emulates the classic hex-dump inspector, generating binary matrix hexadecimal tracks or readable printable ASCII sweeps. |
| `IMVIEW <img_path>`| Multimedia | Connects with hardware JPEG decoders to stream high-resolution graphics straight onto display buffers. |

---

## 🛠️ App Ecosystem & Compiling Native Binaries

You can develop high-performance native plugins for MicroDOS inside standard IDEs (like VS Code) using the provided PlatformIO SDK layout structure.

### 1. App SDK Framework (`microdos_api.h`)
Native applications interface with the operating system through a unified BIOS jump table vector map. It features `__attribute__((weak))` overrides to ensure you can use standard memory structures safely without bloating the plugin size:

```c
#include "microdos_api.h"

// Entrypoint signature requested by the MicroDOS Dynamic Relocation Loader
int _start(int argc, char** argv, MicroDosAPI* api) {
    _global_api_ptr = api; // Bind memory allocation hooks back to OS core
    
    api->clear();
    api->println(STRING("--- Dynamic C Guest Application Live ---"));
    api->beep(440, 250);
    
    // Launch dynamic 4-bit graphical loop matrix
    api->initGameMatrix();
    api->rect(10, 10, 100, 50, GREEN);
    api->flushGameMatrix();
    
    api->delay(2000);
    api->closeGameMatrix();
    return 0; // Returns exit status codes back up safely to Host Core tasks
}
```

### 2. Toolchain Compilation Directives (`platformio.ini`)
To build plugins that can survive runtime relative offset assignments, use these exact compilation flags:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
build_flags = 
    -O2
    -fpack-struct=4
    -fPIC                               ; Position Independent Code flag
    -mlongcalls                         ; Issue far assembly call strings
    -fno-jump-tables                    ; Suppress absolute optimization lookups
    -nostartfiles                       ; Strip standard boot architecture
    -nodefaultlibs                      ; Strip default system library footprint
    -Wl,-e,_start                       ; Map entry target strictly to start pointer
    -Wl,-T,mdb.ld                       ; Enforce custom layout linker map script
    -mtext-section-literals             ; Interleave literals to maintain L32R safety limits
extra_scripts = post:extract_bin.py     ; Extract packaged segment tracks automatically
```

---

## 🚀 Getting Started

### Hardware Requirements
* **ESP32 Cheap Yellow Display (CYD) Clone** (E32N40T) OR **JC3248 capacitive variant**.
* Micro SD card (FAT32 formatted).
* *Optional:* A `/WIFI.CFG` file stored on the SD root directory containing your network credentials (SSID on line 1, password on line 2) to unlock automated fallback network card provisioning.

### Flashing the OS
1. Clone this repository down to your computer workspace.
2. Open the host workspace directory using **PlatformIO**.
3. Select your hardware target environment layout profile flags inside the main configuration files (`BOARD_CYD` or `BOARD_JC3248`).
4. Connect your hardware target over USB, build, and flash.

---

## 📄 License
This project is open-source software licensed under the terms of the **MIT License**. Check out the `LICENSE` file template for comprehensive legal guidelines.
