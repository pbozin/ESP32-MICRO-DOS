## 📚 Table of Contents
---

- [📁 Group 1: Console, Terminal & Text Operations](#-group 1-console-terminal--text-operations)
- [🎨 Group 2: Low-Level Vector Graphics](#-group-2-low-level-vector-graphics)
- [⌨️ Group 3: Embedded Hardware Signals & Input Engine](#️-group-3-embedded-hardware-signals--input-engine)
- [🧠 Group 4: Low-Level Memory Sandboxing & Direct GPIO Fences](#-group-4-low-level-memory-sandboxing--direct-gpio-fences)
- [🖼️ Group 5: Extended Visual Components & Dynamic Allocator Hooks](#️-group-5-extended-visual-components--dynamic-allocator-hooks)
- [🎮 Group 6: Sprite Graphics & Game Engine Architecture](#-group-6-sprite-graphics--game-engine-architecture)
- [🔌 Group 7: Hardware Serial Port Communications (UART2) & Storage Management](#-group-7-hardware-serial-port-communications-uart2--storage-management)
- [🌐 Group 8: Network Operations, AI Integration & System Diagnostics](#-group-8-network-operations-ai-integration--system-diagnostics)

---

## 📁 Group 1: Console, Terminal & Text Operations

### print
```cpp
void print ( const char* text );
```
#### Print string to console
Writes the character string pointed to by `text` directly to the active console terminal output. Text wraps automatically if it exceeds the terminal width layout boundaries.

#### Parameters
* **text**: Null-terminated C-string containing the characters to be displayed on the screen.

#### Return Value
* None.

---

### println
```cpp
void println ( const char* text );
```
#### Print string to console with line feed
Writes the character string pointed to by `text` to the active console terminal output, immediately followed by a carriage return and line feed (`\r\n`) to advance the cursor to the next line.

#### Parameters
* **text**: Null-terminated C-string containing the line of text to be displayed.

#### Return Value
* None.

---

### clear
```cpp
void clear ();
```
#### Clear screen canvas
Clears the active console screen layout buffer completely and resets the interior cursor position back to home base coordinates `(0, 0)`. It fills the screen area with the active background palette color index.

#### Parameters
* None.

#### Return Value
* None.

---

### printAt
```cpp
void printAt ( int x, int y, const char* text );
```
#### Print string at grid coordinates
Renders the character string pointed to by `text` at a specific text grid row and column coordinate mapping on the terminal matrix interface.

#### Parameters
* **x**: The character column position index on the display grid.
* **y**: The character row position index on the display grid.
* **text**: Null-terminated C-string containing the character payload to render.

#### Return Value
* None.

---

### inputStr
```cpp
void inputStr ( const char* prompt, char* destBuffer, int maxLen );
```
#### Read interactive string from terminal
Displays the text prompt string and pauses execution to accept string character input from the system's active input interface. The function handles interactive visual rendering and cursor management within a safe, fixed-size bounds constraint.

#### Parameters
* **prompt**: Null-terminated C-string printed to the user prior to capturing input.
* **destBuffer**: Pointer to an allocated character array where the inputted string will be safely stored.
* **maxLen**: The maximum upper-limit number of characters allowed to be written into `destBuffer` to prevent stack overflows.

#### Return Value
* None.

---

### setTextColor
```cpp
void setTextColor ( int colorId );
```
#### Set default terminal text color
Updates the standard foreground drawing color index used by all sequential text rendering routines.

#### Parameters
* **colorId**: An integer ranging from `0` to `15` corresponding to the 4-bit standard color palette structure.

#### Return Value
* None.

---

### setBgColor
```cpp
void setBgColor ( int colorId );
```
#### Set default terminal background color
Updates the active background text grid fill color index used during sequential print steps and screen clear execution operations.

#### Parameters
* **colorId**: An integer matching a 4-bit index color (`0`–`15`).

#### Return Value
* None.

---

### getTextColor
```cpp
int getTextColor ();
```
#### Get current text color index
Retrieves the active global system foreground 4-bit color palette slot index currently configured for text rendering operations.

#### Parameters
* None.

#### Return Value
* Returns an integer mapping to the current active text palette index slot.

---

### getBgColor
```cpp
int getBgColor ();
```
#### Get current background color index
Retrieves the active global system background 4-bit color palette slot index currently used for backing text prints and canvas draws.

#### Parameters
* None.

#### Return Value
* Returns an integer representing the active background palette slot index.

---

## 🎨 Group 2: Low-Level Vector Graphics

### plot
```cpp
void plot ( int x, int y, int colorId );
```
#### Plot pixel on canvas
Plots a single geometric pixel at the specified coordinate directly onto the active hardware video canvas display layer.

#### Parameters
* **x**: The raw horizontal pixel coordinate on the display matrix.
* **y**: The raw vertical pixel coordinate on the display matrix.
* **colorId**: An integer value (`0`–`15`) indexing into the active 4-bit standard system color palette structure.

#### Return Value
* None.

---

### drawLine
```cpp
void drawLine ( int x1, int y1, int x2, int y2, int colorId );
```
#### Draw line between points
Renders a 1-pixel wide continuous geometric vector line spanning between two terminal coordinates using a hardware-optimized version of Bresenham's line algorithm.

#### Parameters
* **x1 / y1**: The starting point horizontal and vertical coordinates.
* **x2 / y2**: The terminating point horizontal and vertical coordinates.
* **colorId**: 4-bit palette identifier index color (`0`–`15`).

#### Return Value
* None.

---

### drawRect
```cpp
void drawRect ( int x, int y, int w, int h, int colorId );
```
#### Draw wireframe rectangle outline
Draws a hollow rectangular layout profile on the current frame canvas layer without altering any interior pixels.

#### Parameters
* **x / y**: Top-left spatial bounding coordinates.
* **w**: Layout width span constraint in canvas pixels.
* **h**: Layout height span constraint in canvas pixels.
* **colorId**: 4-bit palette identifier index color (`0`–`15`).

#### Return Value
* None.

---

### fillRect
```cpp
void fillRect ( int x, int y, int w, int h, int colorId );
```
#### Draw solid filled rectangle
Renders a solid, completely filled rectangular block layout onto the active video coordinate surface matrix.

#### Parameters
* **x / y**: Top-left spatial bounding coordinates.
* **w**: Layout width span constraint in canvas pixels.
* **h**: Layout height span constraint in canvas pixels.
* **colorId**: 4-bit palette identifier index color (`0`–`15`) used to fill the layout block.

#### Return Value
* None.

---

### drawCircle
```cpp
void drawCircle ( int x, int y, int r, int colorId );
```
#### Draw circle outline
Draws a hollow circular outline configuration using an optimized midpoint mathematical circle rendering routine.

#### Parameters
* **x / y**: Spatial center grid coordinates of the target circle.
* **r**: Radius parameter length measurement defined in raw pixels.
* **colorId**: 4-bit palette identifier index color (`0`–`15`).

#### Return Value
* None.

---

### fillCircle
```cpp
void fillCircle ( int x, int y, int r, int colorId );
```
#### Draw solid filled circle
Draws a solid, completely filled geometric circle layout matching the designated center point and outer radius constraints.

#### Parameters
* **x / y**: Spatial center grid coordinates of the target circle.
* **r**: Radius parameter length measurement defined in raw pixels.
* **colorId**: 4-bit palette identifier index color (`0`–`15`) used to solid-fill the geometry.

#### Return Value
* None.

---

## ⌨️ Group 3: Embedded Hardware Signals & Input Engine

### beep
```cpp
void beep ( int freq, int ms );
```
#### Pulse audio buzzer tone
Generates an explicit acoustic signal tone at the chosen frequency pulse rate through the physical hardware buzzer circuit interface. This execution runs in a non-blocking configuration.

#### Parameters
* **freq**: Sound wave frequency measurement specified in Hertz (Hz).
* **ms**: Continuous sound duration threshold quantified in milliseconds.

#### Return Value
* None.

---

### delay
```cpp
void delay ( int ms );
```
#### Yield system task execution
Pauses program thread operations for the designated duration. This wrapper calls the internal FreeRTOS scheduler to context-switch execution and let low-level background daemons execute smoothly without causing Watchdog Timer (WDT) trips.

#### Parameters
* **ms**: Total pause duration threshold quantified in milliseconds.

#### Return Value
* None.

---

### inkey
```cpp
int inkey ();
```
#### Poll non-blocking keyboard state
Queries the system matrix hardware layout to detect whether any specific interface button is currently being engaged by the operator. It instantly processes events without halting linear sequence workflows.

#### Parameters
* None.

#### Return Value
* Returns an integer character token code matching the pressed button key. If no active input state change is currently detected, it returns `0`.

---

### getTouch
```cpp
void getTouch ( TouchState* state );
```
#### Read touch panel digitizer state
Extracts real-time coordinate state captures directly from the active onboard touch screen digitizer hardware controller chip over the physical connection bus.

#### Parameters
* **state**: Pointer to an allocated `TouchState` data structure configuration which will be populated with the current boolean validation state and calculated coordinates.

#### Return Value
* None.

---

### setKeyboardShift
```cpp
void setKeyboardShift ( int state );
```
#### Toggle input shift tracking state
Manually overwrites or adjusts the interior tracking flag configuration governing whether lowercase values or capital symbol inputs are natively captured.

#### Parameters
* **state**: An integer where `1` forces the active shift modifier flag into a high state, and `0` resets input capture routines back to baseline standard values.

#### Return Value
* None.

---

### getKeyboardShift
```cpp
int getKeyboardShift ();
```
#### Check active shift tracking state
Inspects the current system state configuration tracking variables to report whether an operational shift state override behavior is active.

#### Parameters
* None.

#### Return Value
* Returns an integer state boolean where `1` registers an active shift modification status and `0` indicates a baseline character profile.

---

## 🧠 Group 4: Low-Level Memory Sandboxing & Direct GPIO Fences

### peek
```cpp
int peek ( int address );
```
#### Read value from system RAM address
Inspects a specific direct virtual hardware address slot within the internal array configuration space to retrieve its integer value entry.

#### Parameters
* **address**: The indexed linear target integer address location matching a slot inside the sandboxed hardware tracking array.

#### Return Value
* Returns the integer value currently recorded at the designated storage address boundary.

---

### poke
```cpp
void poke ( int address, int value );
```
#### Write value to system RAM address
Modifies a specific direct virtual hardware address slot within the internal sandboxed tracking boundary, overwriting its value parameter with new data.

#### Parameters
* **address**: The indexed linear target integer address location to overwrite.
* **value**: The new integer variable payload value data to write into the targeted index slot.

#### Return Value
* None.

---

### getRamSize
```cpp
int getRamSize ();
```
#### Get total sandboxed RAM size
Queries the core operating engine setup bounds configuration to read out the maximum total element storage allocation capability available inside the memory arrays.

#### Parameters
* None.

#### Return Value
* Returns an integer representing the maximum memory address limits available for processing allocation routines.

---

### pinMode
```cpp
void pinMode ( int pin, int mode );
```
#### Set protected GPIO pin mode
Configures an available physical interface pin layout connection structure map option to handle dedicated inputs or hardware driver operations. Protected pin checking guards against core device bus disruptions.

#### Parameters
* **pin**: The target interface pin identifier value mapping to an external connection port configuration.
* **mode**: The technical operational interface property behavior option (e.g., `INPUT`, `OUTPUT`, `INPUT_PULLUP`, `INPUT_PULLDOWN`).

#### Return Value
* None.

---

### digitalWrite
```cpp
void digitalWrite ( int pin, int val );
```
#### Write logic state to GPIO pin
Drives the logical voltage layout potential of a validated, non-protected interface pin map option high or low.

#### Parameters
* **pin**: The target physical peripheral pin identifier mapping value.
* **val**: The target logical signal voltage configuration layout instruction value (`1` for high, `0` for low).

#### Return Value
* None.

---

### digitalRead
```cpp
int digitalRead ( int pin );
```
#### Read logic state from GPIO pin
Samples the logical signal voltage state value present on a validated target peripheral interface pin map entry.

#### Parameters
* **pin**: The target physical electronic interface pin map value.

#### Return Value
* Returns an integer reporting the real-time sampled logic state value (`1` for high, `0` for low). Returns `-1` if the pin fails safety checks.

---

## 🖼️ Group 5: Extended Visual Components & Dynamic Allocator Hooks

### drawJpeg
```cpp
void drawJpeg ( const char* filename, int x, int y );
```
#### Render JPEG image to screen
Decodes a standard JPEG image file located on the storage media and renders it directly onto the active video display canvas using an optimized hardware-assisted transformation routing matrix.

#### Parameters
* **filename**: Null-terminated C-string specifying the absolute file path to the target image resource on disk.
* **x**: Target horizontal pixel landing offset position on the screen.
* **y**: Target vertical pixel landing offset position on the screen.

#### Return Value
* None.

---

### setFKeys
```cpp
void setFKeys ( const char* l1, const char* l2, const char* l3, const char* l4, const char* l5 );
```
#### Configure custom soft function keys
Overwrites the text labels assigned to the screen's bottom soft function key navigation tray rows, altering their interactive runtime command mappings dynamically.

#### Parameters
* **l1 / l2 / l3 / l4 / l5**: Null-terminated C-strings representing the five custom text labels to draw across the interface deck.

#### Return Value
* None.

---

### clearFKeys
```cpp
void clearFKeys ();
```
#### Reset soft function keys to default
Wipes out all dynamic macro button labels and restores the default soft function navigation layout mappings (`<-`, `->`, `ESC`, `DEL`, `<X`) to the active bottom keyboard drawer display tray.

#### Parameters
* None.

#### Return Value
* None.

---

### malloc
```cpp
void* malloc ( unsigned int size );
```
#### Allocate dynamic heap memory block
Dynamically provisions a contiguous chunk of uninitialized memory from the internal system heap sandboxing structure allocations.

#### Parameters
* **size**: The total number of consecutive byte structures requested for active program thread runtime allocation operations.

#### Return Value
* Returns a void pointer targeting the start of the newly reserved storage boundary slot block. Returns a null pointer pointer (`0`) if the heap allocation request fails.

---

### free
```cpp
void free ( void* ptr );
```
#### Release allocated heap memory block
Relinquishes an actively claimed runtime memory block back to the global unallocated allocator tracking pools to prevent application memory leaks.

#### Parameters
* **ptr**: Void pointer pointing to the starting memory address of a valid, pre-allocated block previously instantiated by a `malloc` assignment sequence.

#### Return Value
* None.

---

## 🎮 Group 6: Sprite Graphics & Game Engine Architecture

### createSprite
```cpp
uint32_t createSprite ( const char* filename, int spriteSize );
```
#### Load and initialize an RLE sprite asset
Loads a custom 4-bit graphical sprite file from disk storage and parses its compressed RLE runtime frame sequence parameters into system memory allocations.

#### Parameters
* **filename**: Null-terminated C-string specifying the file path to the target sprite resource.
* **spriteSize**: The square bounding dimensions (width and height) of the graphical sprite asset layout.

#### Return Value
* Returns a unique `uint32_t` handle identification token mapping to the loaded sprite management structure. Returns `0` if file tracking operations fail.

---

### drawSprite
```cpp
void drawSprite ( uint32_t spriteHandle, int x, int y, int color, int frame );
```
#### Render a sprite frame onto the display matrix
Renders a specific animation index frame of a registered sprite asset directly onto the designated display interface coordinates.

#### Parameters
* **spriteHandle**: The unique pointer token generated during initial resource creation steps.
* **x / y**: Bounding coordinate target layout variables mapping on the screen grid.
* **color**: Chromatic tint configuration adjustments or index parameters targeting palette layers.
* **frame**: The specific structural zero-indexed animation row to extract from the parsed sprite resource data.

#### Return Value
* None.

---

### clearSprite
```cpp
void clearSprite ( uint32_t spriteHandle );
```
#### Erase sprite background footprints
Wipes out visual artifacts or residual footprint tracking maps left behind by dynamic sprite translation steps.

#### Parameters
* **spriteHandle**: The active targeting pointer handle identifying the asset reference struct.

#### Return Value
* None.

---

### freeSprite
```cpp
void freeSprite ( uint32_t spriteHandle );
```
#### Release sprite resources from memory
Deallocates all internal frame array trackers and memory slots assigned to the target sprite token, returning the memory block to the system pools.

#### Parameters
* **spriteHandle**: The target pointer token identifying the sprite asset structure to delete.

#### Return Value
* None.

---

### initGameMatrix
```cpp
int initGameMatrix ();
```
#### Initialize double-buffered game canvas
Allocates and locks an offscreen canvas matrix buffer layer to enable fast flicker-free sprite transforms, moving graphics processing into a dedicated gaming frame cycle.

#### Parameters
* None.

#### Return Value
* Returns an integer status token where `1` indicates successful canvas space instantiation and `0` indicates initialization failure.

---

### flushGameMatrix
```cpp
void flushGameMatrix ();
```
#### Commit offscreen canvas buffer to screen
Pushes the contents of the offscreen canvas tracking buffer layout instantly onto the physical hardware panel via DMA or high-speed SPI bus transfers.

#### Parameters
* None.

#### Return Value
* None.

---

### closeGameMatrix
```cpp
void closeGameMatrix ();
```
#### Terminate game matrix graphics mode
Closes out active double-buffer frame logic configurations and destroys the temporary graphic canvas, restoring standard console line printing layers.

#### Parameters
* None.

#### Return Value
* None.

---

### createCollisionMap
```cpp
uint32_t createCollisionMap ( int x, int y, int w, int h, int color, int finalCols, int finalRows, int* map );
```
#### Generate logical grid collision grid array
Analyzes coordinate space parameters inside a specific bounded region to look for hitting palette indices, rendering a binary bit-map grid mask out into local pointer vectors.

#### Parameters
* **x / y**: Top-left bounds origin parameters tracking on the active display matrix.
* **w / h**: Total rectangular width and height dimension check constraints.
* **color**: The target index color value to evaluate as a blocking object boundary layer.
* **finalCols / finalRows**: Target dimensions of the generated logical lookup grid mapping structure.
* **map**: Pointer array targeting an allocated destination space intended to hold the processed data calculations.

#### Return Value
* Returns a verification token status value confirming map validation properties.

---
## 🔌 Group 7: Hardware Serial Port Communications (UART2) & Storage Management

This section covers functions for managing the secondary hardware serial port (UART2) and storage operations (SD card file handling).

### Functions Overview
#### **Serial Functions:** `serialOpen`, `serialWrite`, `serialRead`, and `serialClose` handle initialization, data transmission, non-blocking reading, and resource release for the UART2 interface.
* **`serialOpen(uint32_t baud, int txPin, int rxPin)`**: Initializes UART2 with specified speed and pins, returning `0` on success or `-1` if protected.
* **`serialWrite(const uint8_t* buffer, unsigned int length)`**: Transmits raw binary data arrays over the active UART connection.
* **`serialRead(uint8_t* buffer, unsigned int maxLength)`**: Non-blocking read from the UART input queue, returning the byte count, `0`, or `-1` on error.
* **`serialClose()`**: Shuts down the secondary UART module and frees pins back to GPIO.

#### **Storage Functions:** `sdOpen`, `sdRead`, `sdWrite`, and `sdClose` manage file streams, reading, writing, and proper closure/flushing on SD media.
* **`sdOpen(const char* filename, const char* mode)`**: Opens an asset path on mounted storage, returning a file handle or null (`0`) on failure.
* **`sdRead(void* fileHandle, void* buffer, uint32_t size, uint32_t count)`**: Reads data blocks from an open file stream into a destination buffer.
* **`sdWrite(void* fileHandle, const void* buffer, uint32_t size, uint32_t count)`**: Writes data blocks from a buffer to the storage media file stream.
* **`sdClose(void* filehandle)`**: Flushes data, saves metrics, and closes the active file stream.

---

## 🌐 Group 8: Network Operations, AI Integration & System Diagnostics

### wifiUp
```cpp
int wifiUp ( const char* ssid, const char* pass );
```
#### Initialize Wi-Fi station connectivity
Activates the internal wireless network chip in station mode and attempts to log onto the designated local access point network route path.

#### Parameters
* **ssid**: Null-terminated C-string containing the targeting network broadcast identity name.
* **pass**: Null-terminated C-string containing the authentication key passphrase credentials.

#### Return Value
* Returns an integer status verification token where `1` signals a successfully authenticated connection state and `0` indicates connection tracking errors.

---

### wifiDown
```cpp
void wifiDown ();
```
#### Deactivate network interface radios
Disconnects from the active network station route path and powers down internal wireless transmission circuitry to conserve power.

#### Parameters
* None.

#### Return Value
* None.

---

### ollamaStream
```cpp
int ollamaStream ( const char* prompt, const char* serverIp, const char* modelName, const char* sysPrompt, char* outBuf, int outBufSize, int streamToConsole );
```
#### Stream payload request to local Ollama AI host
Funnels a JSON request string payload over a network socket to a target local network Ollama LLM execution node server instance.

#### Parameters
* **prompt**: Null-terminated C-string detailing the primary interrogation text instructions.
* **serverIp**: Complete network string detailing targeting destination endpoint address paths.
* **modelName**: Targeting model profile system execution tag label.
* **sysPrompt**: Baseline formatting rules configuration instructions passed down to structural system prompts.
* **outBuf**: Destination target buffer space allocated to hold incoming character text stream strings.
* **outBufSize**: Allocated boundary length constraint variable checking limits on `outBuf`.
* **streamToConsole**: Boolean option token where passing `1` mirrors network characters to the shell output layout terminal space.

#### Return Value
* Returns an execution confirmation code token indicating connection stability properties.

---

### random
```cpp
int random ( int min, int max );
```
#### Generate pseudo-random integer value
Evaluates an internal running formula seed to generate a numeric value within a specific constraint range.

#### Parameters
* **min**: The inclusive lower constraint boundary tracking limits on the generated integer value.
* **max**: The exclusive upper constraint boundary tracking limits on the generated integer value.

#### Return Value
* Returns an integer falling within the configured bounding range.

---

### sysDebugDump
```cpp
void sysDebugDump ( const char* label, const void* memoryAddress, unsigned int byteCount, uint32_t virtualAddr );
```
#### Execute memory address hex visualization log dump
Prints out structured diagnostic information targeting raw address blocks. It outputs data into standard formatted hexadecimal logs on the debugging serial connection line.

#### Parameters
* **label**: Heading text tracking log session identities.
* **memoryAddress**: Void pointer targeting the internal storage boundary layout layer to inspect.
* **byteCount**: Total consecutive address byte steps to print out onto output streams.
* **virtualAddr**: A proxy logical label reference string mapping tracking values to output logs.

#### Return Value
* None.

