# ESP32-MICRO-DOS Utility Library Reference Manual (`microdos_util.h`)

This library provides a subset of custom-tailored standard C library utilities optimized for performance and lower footprint over standard toolchain inclusions.

---

## 🧠 Memory Block Operations

### memcpy
```cpp
void* memcpy ( void* dest, const void* src, unsigned int n );
```
#### Copy block of memory
Copies `n` bytes from the location pointed to by `src` directly to the memory block pointed to by `dest`. Includes an optimized fast-path path that transfers 32-bit words simultaneously if the memory addresses are 4-byte aligned.

#### Parameters
* **dest**: Pointer to the destination array where the content is to be copied.
* **src**: Pointer to the source of data to be copied.
* **n**: Number of bytes to copy.

#### Return Value
* Returns the destination pointer (`dest`).

---

### memset
```cpp
void* memset ( void* dest, int value, unsigned int count );
```
#### Fill block of memory
Fills the first `count` bytes of the memory area pointed to by `dest` with the constant byte `value`. Optimizes handling into 32-bit word fills if both the pointer and byte count match 4-byte address bounds requirements.

#### Parameters
* **dest**: Pointer to the block of memory to fill.
* **value**: Value to be set. The value is passed as an int, but the function fills the block using the unsigned char conversion of this value.
* **count**: Number of bytes to be set to the value.

#### Return Value
* Returns the destination pointer (`dest`).

---

## 🔢 Data Conversion Utilities

### atoi
```cpp
int atoi ( const char* str );
```
#### Convert string to integer
Parses the C-string `str`, interpreting its content as an integer numeric value, skipping initial whitespace blocks and handling trailing sign specifiers (`+` or `-`).

#### Parameters
* **str**: Null-terminated C-string beginning with an optional numeric representation.

#### Return Value
* Returns the parsed integer value. Returns `0` if no valid conversion can be performed.

---

### itoa
```cpp
void itoa ( uint32_t val, char* buf );
```
#### Convert unsigned integer to string
Converts an unsigned 32-bit integer value into a null-terminated decimal radix character array string representation.

#### Parameters
* **val**: Unsigned 32-bit integer value to be converted.
* **buf**: Target character array buffer where the resulting string will be stored.

#### Return Value
* None.

---

### ftoa
```cpp
void ftoa ( float in, char* out, int outLen, int precision );
```
#### Convert floating-point value to string
Converts a single-precision floating point number into a formatted decimal representation, automatically structuring signage, rounding limits, and fractional padding metrics.

#### Parameters
* **in**: Floating-point value to convert.
* **out**: Target character string array space allocation intended to receive the formatted payload.
* **outLen**: Total available byte length constraint sizing allowed inside the `out` array buffer.
* **precision**: The fixed number of structural digits to display following the decimal point marker.

#### Return Value
* None.

---

## 🧵 String Manipulation Primitives

### strlen
```cpp
unsigned int strlen ( const char* str );
```
#### Get string length
Calculates the exact sequence length of the null-terminated string `str`, excluding the final null character itself.

#### Parameters
* **str**: Null-terminated C-string to examine.

#### Return Value
* Returns the total unsigned integer character length count.

---

### padString
```cpp
void padString ( char* dest, const char* src, size_t fixedLen );
```
#### Copy string and pad to fixed length
Copies characters from `src` to `dest`. If the source string is shorter than `fixedLen`, the function pads the remaining space with blank space characters (`' '`) up to the specified length threshold.

#### Parameters
* **dest**: Pointer to the destination buffer array (must be at least `fixedLen + 1` bytes wide).
* **src**: Null-terminated source string payload.
* **fixedLen**: The strict target string length boundary layout parameter to enforce.

#### Return Value
* None.

---

### Quick Reference: Other String Standard Primitives
* **`strcpy(char* dest, const char* src)`**: Copies the source string onto the destination memory limits.
* **`strncpy(char* dest, const char* src, size_t n)`**: Copies up to `n` bytes from a source string.
* **`strcat(char* dest, const char* src)`**: Appends a copy of the source string onto the end of the destination string.
* **`concat(const char* first, const char* second, char* result)`**: Concatenates two strings sequentially into a pre-allocated destination buffer, returning a tracking pointer.
* **`strcmp(const char* s1, const char* s2)`**: Compares two strings lexically, returning `0` if they match perfectly.
* **`strncmp(const char* s1, const char* s2, size_t n)`**: Compares up to `n` bytes of two strings.
* **`reverse_str(char* str, int len)`**: Performs an in-place string flip inversion up to the defined boundary constraint variable `len`.
