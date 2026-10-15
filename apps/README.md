# MicroDOS Native Application Registry

This directory houses the standalone, native C production components compiled via the **MicroDOS App SDK** (`api/include/microdos_*.h`). These programs showcase how to use the dynamic memory sandbox, vector graphics engine, and streaming LLM token processors.

---

## 🎮 1. Arcade Brick Breaker (`bricks.bin`)
A high-frame-rate breakout simulator showcasing graphics, real-time keyboard polling sweeps, and system RAM sandboxing.

### Architectural Blueprint
* **Integrated Memory Mapping (`api->poke` / `api->peek`):** The program maps the block grid by using indices `0` to `49` inside the parent operating system's `systemRAM` array vector. This eliminates local memory tracking variables.
* **Non-Blocking Key Captures (`api->inkey()`):** Samples user clicks during instruction frames. It listens for specific ASCII decimal values to move the paddle without blocking execution tracking loops.
* **Shatter-Block Erasure Loop:** When a collision is identified (`api->peek(n) == 1`), the code marks the coordinates as broken (`api->poke(n, 0)`), issues an inline sound pulse (`api->beep()`), and targets an isolated overwrite box (`api->rect()`) to clear *only* that block, leaving the remaining bricks untouched.

---

## 🧠 2. AI Philosophical Terminal (`chat.bin`)
An interactive chat client that connects your ESP32 terminal to a remote **Ollama Large Language Model** server.

### Architectural Blueprint
* **System Automation Fallback (`api->wifiUp`):** Initializes the internal network stack. Passing placeholder parameters like `STRING("CFG")` causes the kernel to automatically look for local credentials inside `/WIFI.CFG` on the SD card.
* **Token Stream Interceptor:** Connects via `api->ollamaStream()` to point-of-presence servers (defined in /OLLAMA.CFG) running lightweight model (also defined in /OLLAMA.CFG). Tokens are parsed directly from socket streams to conserve RAM.
* **Modal Text Accumulator:** Captures text via `api->inputStr()`, appending prompts cleanly behind `YOU> ` and routing incoming tokens dynamically to stream outputs.

---

## 🤖 3. Autonomous Code Agent Terminal (`code.bin`)
A development pipeline that prompts a remote AI Coder model to write, compile, and stream executable BASIC code blocks directly into the local interpreter.

### Architectural Blueprint
* **Strict Schema Enforcement:** Employs a defensive system configuration prompt that forces the LLM to wrap code outputs inside clean markdown boundaries (```` ```basic ````) and output explicit incremental line numbers.
* **Autonomous Memory Harvesting Engine:** The code feeds prompts through `api->ollamaStream()` to a target model (`qwen2.5-coder-24k:3b`). The underlying OS intercepts the incoming token tracking tracks, filters out conversation text, clears old structures, and pipes incoming code directly into the active instruction buffer stack via `storeLine()`.

---

## 🎮 4. Retro Chess Engine (`chess.bin`)
A compact, highly optimized chess simulator featuring multi-sprite graphics engine, a dual-input control layout, dynamic function key overrides, and an internal Toledo-style look-ahead heuristic evaluator.

### Architectural Blueprint
* **Hybrid Touch & Alphanumeric Input:** Leverages a dual-mode event listener stack. Users can input standard algebraic coordinate strings (e.g., e2e4) over serial/keyboard polling buffers (`api->inkey()`) or click squares directly on the panel utilizing localized display coordinate division math.
* **Persistent BIOS Function-Key Hooks:** Interlaces UI feedback structures cleanly with the host operating system layer by mapping string pointers back to dynamic hardware macros (`api->setFKeys`). This yields responsive rendering adjustments, board configuration shifts (`FLIP`), and multi-state history adjustments (`UNDO`, `NEW`, `QUIT`).
* **Toledo-Heuristic Material Scan Loop:** Implements a standalone evaluation state-machine running over a shared memory index layout (`b[from]`). The logic simulates individual positions, drops illegal tracks via King tracking verification arrays (`isKingUnderAttack`), rewards structural positional ownership over center-ring grids, and calculates real-time coordinate differentials to pass calculated move paths directly back down to target evaluation hooks (`executeToledoMove`).

---

## 🎮 5. Bouncing 3D Wireframe Sphere (`ball.bin`)
A high-frame-rate physics and graphics demonstration showcasing real-time 3D projection mechanics, localized vertex transformations, and kinetic vector line rendering.

### Architectural Blueprint
* **Procedural Vertex Topology Generation:** Dynamically calculates an interconnected latitude-and-longitude map of points using memory-aligned buffers. It stores geometric structures across unified edge arrays and applies transformations.
* **CPU-Driven 3D Projection Pipeline:** Processes raw trigonometric calculations in software via an isolated mathematical transform stack (`m3d_transform_and_project`). It rotates, projects, and scales local tracking spaces directly onto custom screen layouts relative to dynamic angle changes.
* **Kinetic Vector Line Interceptor:** Calculates real-time 2D coordinate offsets based on basic gravity and bounce algorithms. It feeds dynamic positions directly into the line rasterizer loop, mapping multi-colored edge strokes into frame arrays to simulate wireframe objects bouncing smoothly in real-time.

---

## 🌦️ 6. Atmospheric Environmental Dashboard (`sensor.bin`)
A low-level telemetry terminal that executes a custom bit-banged I2C hardware driver protocol stack to extract, compensate, and render real-time climate data streams from a Bosch BME68x sensor.

### Architectural Blueprint
* **Bit-Banged Protocol Layer:** Manages timing-critical register communication loops via inline assemby and software-driven digital pin polling (`digitalRead` / `digitalWrite`). It manually drives clock (`SCL_PIN`) and data (`SDA_PIN`) lines to implement standard START, STOP, and ACK I2C bus signals completely in software outside the hardware peripheral stack.
* **Dual-Region Memory Calibration Unpacker:** Maps factory-fused compensation coefficients by parsing distinct physical sensor registers directly into memory-aligned runtime arrays. This layout prevents multi-byte structures from throwing memory management tracking errors inside the Xtensa core layout.
* **Bosch Fixed-Point Compensation Evaluator:** Decodes multi-byte raw data frames using structural coordinate bitwise shifts and algebraic multiplier formulas. It balances volatile integer values dynamically via internal memory references to convert analog physics registers into human-readable text buffers.

---

## 🎮 7. Real-Time Geometric Morphing Engine (`morph.bin`)
A software-driven 3D vector engine displaying mathematical shape interpolation, dynamic color phase mapping, and automatic topological translation states.

### Architectural Blueprint
* **Multi-State Coordinate Array Interleaving:** Allocates memory-aligned, fixed-point array structures containing unique 3D vertex configurations (Cube, Octahedron, and Tetrahedron). The topologies use identical vector node indices (`wireframeTopology`) to allow structural transitions without reallocating active runtime objects.
* **Dynamic State Linear Interpolation:** Uses a state tracker variable driven by an inversion loop to sweep back and forth between positions. It streams secondary and tertiary matrix data arrays into the software projection engine core (`m3d_transform_and_project`) to calculate intermediate coordinates on the fly.
* **Phase-Mapped Vector Rasterizer:** Monitors real-time state tracking thresholds to inject adaptive color definitions across changing coordinate maps. It passes transformed 2D points into an optimized drawing routine, keeping the line animations crisp and tear-free.

---

## 🎮 8. Game of GO against AI (`go.bin`)
An AI-assisted graphical GO game engine.

### Architectural Blueprint
* **Touch Input:** Users can click square intersections directly on the panel utilizing localized display coordinate division math.
* **Persistent BIOS Function-Key Hooks:** Interlaces UI feedback structures cleanly with the host operating system layer by mapping string pointers back to dynamic hardware macros. This yields responsive multi-state history adjustments (`UNDO`, `NEW`, `PASS`).
* **AI model player** Users is playing against local ollama LLM configured in /OLLAMA.CFG (together with /WIFI.CFG for networking).

---

## 🎵 9. Algorithmic Markov MIDI Generative Sequencer (`midigen.bin`)
A live music composition engine that processes dynamic Markov chains and geometric probability distributions entirely in software to stream real-time multi-channel MIDI note data down the system serial bus.

### Architectural Blueprint
* **Dynamic Software Markov Probability Calculator:** Implements an on-the-fly mathematical matrix engine utilizing custom fixed-point reciprocal operations. This creates balanced weight distributions for generative scale progressions without relying on hardware floating-point acceleration.
* **Algorithmic Musical Topology Scales:** Allocates memory-aligned, multi-byte velocity arrays hosting specialized microtonal musical scales (Minor, Dorian, Abstract, Goa, Aeolian). These structures serve as safe algorithmic look-up tables that translate state adjustments directly into valid pitch outputs.
* **Multi-Voice Serial Streaming Dispatcher:** Evaluates active beat metrics, generating targeted three-byte serial packets. It automatically cycles voice routing through dynamic channel arrays to manage multi-instrument synthesis configurations.

---

## 🛠️ Building & Deployment Guide

Applications are compiled outside the core kernel image using **PlatformIO**:

### 1. Build Compilation Pipeline
1. Verify that your target application source code leverages the 4-byte structural memory alignment helpers (`STRING("...")`) required by the Xtensa layout specifications.
2. Run the compiler toolchain inside your IDE terminals:
   ```bash
   pio run
   ```
3. The integrated post-action python build script (`extract_bin.py`) parses the output `.elf` binaries to generate a clean, header-packed binary file layout (e.g., `bricks_mdos.bin`).

### 2. SD Mounting & Shell Launch Execution
1. Copy your compiled binary file onto a FAT32-formatted Micro SD card, renaming it properly (eg. CHESS.BIN or BRICKS.BIN).
2. In the case of `CHAT.BIN`, `CODE.BIN` and 'GO.BIN', ensure that `/WIFI.CFG` and '/OLLAMA.CFG' files exist on the SD root directory:

  **/WIFI.CFG**

   ```text
   wifi_ssid
   wifi_pass
   ```

   **/OLLAMA.CFG**

   ```text
   host:port
   model_name
   ```
3. Insert the card into your ESP32 display device and power on.
4. Launch your application from the interactive command-line interface using the native file loader command:
   ```bash
   > EXEC CHAT.BIN
   ```

   Alternatively:
   ```bash
   > CHAT
   ```
<img width="240" alt="go" src="https://github.com/user-attachments/assets/b5eedcbd-db0d-42d9-9d5a-5e65efd00053" /> |
<img width="240" alt="chess" src="https://github.com/user-attachments/assets/6d88283a-2952-4353-bf6d-665407f43529" /> |
<img width="240" alt="ollama_c" src="https://github.com/user-attachments/assets/1fdc3b57-a5eb-48b2-96aa-fba67de17762" /> |
<img width="240" alt="bricks" src="https://github.com/user-attachments/assets/097d9da4-8c6c-46db-b757-24c1f5410a33" />

