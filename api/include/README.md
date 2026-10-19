# 📟 ESP32-MICRO-DOS API & Core Library System

Welcome to the low-level programmer's ecosystem for **ESP32-MICRO-DOS**. This framework provides sandboxed access to hardware peripherals, multi-buffered fast 2D and 3D graphics extensions, direct network automation capabilities, and optimized runtime memory utilities.

---

## 🗂️ Component Documentation Index

The firmware development structures are separated into three distinct logical sub-systems. Choose a manual layer to begin development operations:

### 1. 🔌 System Kernel API Wrapper
* **File Reference**: [`README_api.md`](../../docs/README_api.md)
* **Description**: Detailed reference for the global `kernelAPI` (`MicroDosAPI`) struct execution vectors. Bridges your guest runtime scripts directly to physical hardware peripherals.
* **Core Systems Covered**: 
  * Text terminal window layouts and cursor tracking
  * 4-bit palette-swapped primitive drawing routines (Points, lines, rects, circles)
  * Hardware interrupts, timers, non-blocking input engines, and touch digitizers
  * Protected GPIO fences and dynamic heap space memory sandboxing
  * SD/SD_MMC file handlers and double-buffered sprite animation graphic loops
  * Station Wi-Fi connection radios and autonomous local Ollama LLM streaming nodes

### 2. 📐 3D Geometry Processing Extension
* **File Reference**: [`README_3d.md`](../../docs/README_3d.md)
* **Description**: Math engine tailored for lightning-fast transformation math on target microcontrollers without dedicated float processing unit extensions.
* **Core Systems Covered**:
  * Bit-aliasing inverse fast reciprocal (1/x) calculators
  * Polynomial-based fast sine and cosine approximation loops
  * Continuous 3-way linear vertex configuration transitions (`lerp3`)
  * Multi-mesh perspective projection structures matching coordinate screen viewspaces

### 3. 🧠 Low-Footprint Standalone C Utilities
* **File Reference**: [`README_util.md`](../../docs/README_util.md)
* **Description**: Tailored subset of lightweight, memory-safe data allocation, data conversion, and text validation routines with `WEAK` linkage configurations.
* **Core Systems Covered**:
  * 32-bit aligned block copies (`memcpy`) and block fills (`memset`)
  * Custom integer-to-string (`itoa`), string-to-integer (`atoi`), and float-to-string (`ftoa`) layout parsers
  * Boundary-checked string manipulation primitives (`strlen`, `padString`, `strncmp`, `reverse_str`)
