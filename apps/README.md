# MicroDOS Native Application Registry

This directory houses the standalone, native C production components compiled via the **MicroDOS App SDK** (`microdos_api.h`). These programs showcase how to use the dynamic memory sandbox, vector graphics engine, and streaming LLM token processors.

---

## 🎮 1. Arcade Brick Breaker (`bricks.bin`)
A high-frame-rate breakout simulator showcasing hardware vector graphics, real-time keyboard polling sweeps, and system RAM sandboxing.

### Architectural Blueprint
* **Integrated Memory Mapping (`api->poke` / `api->peek`):** The program maps the block grid by using indices `0` to `49` inside the parent operating system's `systemRAM` array vector. This eliminates local memory tracking variables.
* **Non-Blocking Key Captures (`api->inkey()`):** Samples user clicks during instruction frames. It listens for specific ASCII decimal values (KEY_O = `79` for Left, KEY_P = `80` for Right, KEY_Q = `113` to exit) to move the paddle without blocking execution tracking loops.
* **Shatter-Block Erasure Loop:** When a collision is identified (`api->peek(n) == 1`), the code marks the coordinates as broken (`api->poke(n, 0)`), issues an inline sound pulse (`api->beep()`), and targets an isolated overwrite box (`api->rect()`) to clear *only* that block, leaving the remaining bricks untouched.

---

## 🧠 2. AI Philosophical Terminal (`chat.bin`)
An interactive chat client that connects your ESP32 terminal to a remote **Ollama Large Language Model** server.

### Architectural Blueprint
* **System Automation Fallback (`api->wifiUp`):** Initializes the internal network stack. Passing placeholder parameters like `STRING("SSID")` causes the kernel to automatically look for local credentials inside `/WIFI.CFG` on the SD card.
* **Token Stream Interceptor:** Connects via `api->ollamaStream()` to point-of-presence servers (default: `192.168.0.131:11434`) running lightweight models like `qwen2.5-theory-24k:3b`. Tokens are parsed directly from socket streams to conserve RAM.
* **Modal Text Accumulator:** Captures text via `api->inputStr()`, appending prompts cleanly behind `YOU> ` and routing incoming tokens dynamically to stream outputs.

---

## 🤖 3. Autonomous Code Agent Terminal (`code.bin`)
A development pipeline that prompts a remote AI Coder model to write, compile, and stream executable BASIC code blocks directly into the local interpreter.

### Architectural Blueprint
* **Strict Schema Enforcement:** Employs a defensive system configuration prompt that forces the LLM to wrap code outputs inside clean markdown boundaries (```` ```basic ````) and output explicit incremental line numbers.
* **Autonomous Memory Harvesting Engine:** The code feeds prompts through `api->ollamaStream()` to a target model (`qwen2.5-coder-24k:3b`). The underlying OS intercepts the incoming token tracking tracks, filters out conversation text, clears old structures, and pipes incoming code directly into the active instruction buffer stack via `storeLine()`.

---

## 🛠️ Building & Deployment Guide

Applications are compiled outside the core kernel image using **PlatformIO**:

### 1. Build Compilation Pipeline
1. Verify that your target application source code leverages the 4-byte structural memory alignment helpers (`STRING("...")`) required by the Xtensa layout specifications.
2. Select your display framework environment configuration definitions inside `platformio.ini` (`BOARD_CYD` or `BOARD_JC3248`).
3. Run the compiler toolchain inside your IDE terminals:
   ```bash
   pio run
   ```
4. The integrated post-action python build script (`extract_bin.py`) parses the output `.elf` binaries to generate a clean, header-packed binary file layout (e.g., `bricks_mdos.bin`).

### 2. SD Mounting & Shell Launch Execution
1. Copy your compiled binary file onto a FAT32-formatted Micro SD card.
2. In the case of `chat.bin` and `code.bin`, ensure a `/WIFI.CFG` file exists on the SD root directory with your access credentials:
   ```text
   Your_WiFi_SSID
   Your_WiFi_Password
   ```
3. Insert the card into your ESP32 display device and power on.
4. Launch your application from the interactive command-line interface using the native file loader command:
   ```bash
   > EXEC BRICKS.BIN
   ```
