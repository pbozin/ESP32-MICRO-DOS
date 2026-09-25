# MicroDOS App SDK & Linker Specification

This directory contains the developer assets required to compile, build, and link high-performance native applications (`.bin`) that run on top of the **ESP32 MicroDOS** kernel via the `EXEC` utility.

---

## 🔬 Toolchain Linker Architecture (`mdb.ld`)

Developing dynamic binary plugins for a Harvard-architecture microcontroller like the ESP32 introduces rigorous layout obstacles. Under standard compilation methods, the Xtensa core executes the `L32R` (Load 32-Bit Register relative) instruction, which utilizes a narrow relative lookup offset. If an application contains massive sequential code branches, references to string literals or data constants placed at the end of the file will instantly overshoot memory boundaries, throwing a hard processor hardware crash (`Instruction Fetch Address Error`).

### The Layout Partitioning Blueprint
To protect guest binaries against long-call memory traps, MicroDOS applies a specialized layout configuration map via `api/mdb.ld`:

1. **Inline Literal Pool Interleaving:** Rather than aggregating `.literal` records inside a separate trailing partition, the linker interleaves them directly beside code boundaries inside the text layout tracking sections: `*(.literal .text .literal.* .text.*)`. This forces data assets to settle directly alongside matching code blocks, preserving localized `L32R` lookup constraints.
2. **Strict Dual-Zone Segregation:** 
   * **ZONE 1 (IRAM Space):** Houses the instruction entries (`.text`) packed with local literal indices. The loader allocates this segment to the ESP32 instruction bus (`MALLOC_CAP_EXEC`).
   * **ZONE 2 (DRAM Space):** Clusters writeable variables (`.data`), read-only strings (`.rodata`), and the lookup matrix vectors (`.got`). The loader assigns this section safely to data RAM footprints.

### Precompiled Structure Verification Block
Before the `EXEC` loader task schedules execution loops inside FreeRTOS, it verifies the 16-byte packed header structure outputted by the `extract_bin.py` post-build parser script:

| Byte Offset | Structural Token Target | Purpose Description |
| :--- | :--- | :--- |
| `0x00 - 0x03` | `_dram_size` | Size constraints allocated to allocate the data heap payload segment. |
| `0x04 - 0x07` | `_iram_size` | Size profiles requested to inject executable machine instructions. |
| `0x08 - 0x0B` | `_start` | Relative target pointer marking the application entry function loop. |
| `0x0C - 0x0F` | `_got_file_offset` | Boundaries identifying the Global Offset Table for Phase 2 address patching. |

---

## 🛠️ SDK Reference API (`microdos_api.h`)

Native applications hook into parent system calls through an isolated jump table interface. 

### Core API Categories

#### 1. Interactive Text Console Vectors
* `api->print(const char* text)` – Draws a string payload directly into the scrolling text buffer grid.
* `api->println(const char* text)` – Outputs a string tracking payload immediately followed by an absolute carriage return line break.
* `api->clear()` – Wipes both historical terminal buffer layers cleanly and resets absolute coordinate pointers.
* `api->inputStr(const char* prompt, char* destBuffer, int maxLen)` – Locks application execution in a blocking track, prompting an operator for string input using the on-screen soft keyboard.

#### 2. Advanced 4-Bit Game Matrix Canvas
* `api->initGameMatrix()` – Initialized a double-buffered 4-bit indexed canvas footprint mapping perfectly onto `ramOSPalette` color definitions. Wipes existing terminal layers.
* `api->flushGameMatrix()` – Directs a rapid DMA block transaction flushing background canvas assets cleanly onto physical hardware registers.
* `api->closeGameMatrix()` – Safely deletes active graphics memory buffers, reclaiming RAM allocations cleanly.

#### 3. High-Performance RLE Sprite Engine
* `api->createSprite(const char* filename)` – Streams an indexed graphic from the SD card storage into a compressed `OSSprite_t` matrix structure, yielding a distinct handle pointer.
* `api->drawSprite(uint32_t handle, int x, int y)` – Executes an advanced chroma-key overlay routine. Checks for transparent bounding codes (Index 15), processes pixel merges, backs up underlying assets, and draws the output to the active frame compositor.
* `api->freeSprite(uint32_t handle)` – Restores underlying dirty background rectangles and drops sprite tracking records out of memory.

#### 4. Vector Geometry Drawing Directives
*Drawing operations apply across `api->color(int colorId)` mappings (Indices 0 through 15).*
* `api->plot(int x, int y, int colorId)` – Directs individual pixel assignments.
* `api->line(int x1, int y1, int x2, int y2, int colorId)` – Constructs clean vector paths.
* `api->rect(int x, int y, int w, int h, int colorId)` – Generates structured solid rectangular block boundaries.
* `api->circle(int x, int y, int r, int colorId)` – Processes standard midpoint circle pixel allocations.

#### 5. Embedded I/O Controls & Protected Fences
* `api->beep(int freq, int ms)` – Triggers square wave alert sequences natively over the audio output bus.
* `api->delay(int ms)` – Releases execution windows cleanly back down to underlying system schedulers.
* `api->inkey()` – Runs a rapid, non-blocking scan tracking active input matrix key coordinates.
* `api->pinMode(int pin, int mode)` / `api->digitalWrite` / `api->digitalRead` – Intercepts external pin interactions, applying strict security guards across crucial display register ports while permitting access to open peripheral networks.

#### 6. Micro-Allocator Hooks
* `api->malloc(unsigned int size)` – Channels requests back into parent kernel allocators, tracking layouts to mitigate memory leaks.
* `api->free(void* ptr)` – Discards dynamic footprints seamlessly.

#### 7. Local AI Agent Streams
* `api->ollamaStream(const char* prompt, const char* ip, const char* model, const char* sysPrompt, int streamToConsole)` – Forwards structured network requests to local Ollama targets. Supports full byte streaming, text backslash extraction parsing, context indexing rolling, and standard display mapping.

---

## 📐 Safe Alignment Macro Rules

Because Xtensa architectures demand absolute data alignment patterns to block runtime execution exceptions, always wrap raw string parameters within your binaries using these integrated helper macros:

```c
// ❌ WRONG: Will trigger an immediate processing exception error!
api->print("Starting Subsystem Check...");

//  RIGHT: Aligns strings cleanly to 4-byte structural bounds
api->print(STRING("Starting Subsystem Check..."));

//  RIGHT: Allocates distinct global constant arrays correctly
NEW_STRING(mySystemLog, "Booting Complete.");
api->println(mySystemLog);
```
