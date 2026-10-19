# ESP32-MICRO-DOS 3D Extension Reference Manual (`microdos_3d.h`)

This extension library provides hardware-optimized, inline 3D graphics mathematics functions designed to bypass traditional performance bottlenecks on embedded targets.

---

## 📐 Data Type Structures

* **`Vector3D`**: Defines a 3D Euclidean coordinate point with `x`, `y`, and `z` float parameters.
* **`Vector2D`**: Defines a 2D screenspace pixel coordinate with integer `x` and `y` parameters.
* **`Edge3D`**: Represents a structural edge connecting two point indices in a vertex buffer array.

---

## 🧮 Fast Hardware Mathematics Modules

* **`m3d_reciprocal(float x)`**: Computes a fast inverse reciprocal (\(1/x\)) approximation using bit-stripping and Newton-Raphson passes. Clamps to `0.001f` if `x` is `0.0f` to prevent division-by-zero.
* **`m3d_sinf(float x)`**: Calculates a fast sine approximation in radians using memory-aliased polynomial constants without standard `math.h` overhead.
* **`m3d_cosf(float x)`**: Calculates a fast cosine approximation by phase-shifting the input through the `m3d_sinf` pipeline.
* **`m3d_lerp3(float v1, float v2, float v3, float state)`**: Performs a 3-way continuous linear interpolation across three anchor values based on a `state` progress modifier (\(0.0f\) to \(2.0f\)).

---

## 🎥 Core 3D Graphics Pipeline Engine

* **`m3d_transform_and_project(...)`**: Executes an optimized rendering pipeline that simultaneously interpolates vertex configurations between three 3D shapes, applies Euler rotations (\(a_x, a_y, a_z\)), and projects the results onto 2D display coordinates stored in `outputPoints`.
