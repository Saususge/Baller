# Baller

### Simple. Fast. Vulkan-oriented 2D engine.

**C++17 / Vulkan 1.4 / Slang / Windows**

A solo engine project built from the first pixel up. Baller aims for a small, readable codebase, efficient sprite rendering, and a playable 2D demo backed by performance measurements.

**Status: early development.** Window and Vulkan instance setup are in place. Shaders compile at build time; the first rendered triangle is the next milestone. Speed is a design goal, with benchmarks still ahead.

[Get started](#get-started) · [Shaders](#shaders) · [Roadmap](#roadmap)

---

## In the engine today

- Win32 window creation and event handling.
- Vulkan 1.4 instance setup, loader version checks, validation layers, and surface creation.
- Slang vertex and fragment shaders compiled to SPIR-V through CMake.
- Visual Studio presets for Windows x64 Debug and Release builds.

GPU selection, the logical device, swapchain, and graphics pipeline are still to come. The current application opens a window; it does not render the triangle yet.

## Get started

### Requirements

- Windows x64 and a GPU/driver that supports Vulkan 1.4.
- Visual Studio 2022 or newer. Development currently uses Visual Studio 2026.
- The **Desktop development with C++** workload, including:
  - MSVC x64/x86 build tools.
  - Windows 10 or Windows 11 SDK.
  - C++ CMake tools for Windows: CMake 3.24+ and Ninja.
- [Vulkan SDK](https://vulkan.lunarg.com/) 1.4+ with the **Slang compiler** (`slangc.exe`).
- Git. CMake fetches GLM 1.0.1 on the first configure if no installed GLM package is found.

Restart Visual Studio after installing the Vulkan SDK so it picks up `VULKAN_SDK`. The SDK version and the GPU driver's supported Vulkan version are separate requirements.

### Open. Build. Run.

1. In Visual Studio, choose **File → Open → Folder** and select this repository.
2. Select **baller | Windows x64 Debug** or **baller | Windows x64 Release**.
3. Wait for CMake configuration, then select **baller.exe** as the startup item.
4. Build and press **F5**. Press **ESC** to exit.

The presets use Ninja with MSVC x64. Shader compilation is part of the `baller` build. Slang sources appear in the target's **Shaders** group; syntax highlighting and completion depend on IDE extensions.

### From the terminal

Use an **x64 Native Tools Command Prompt for VS** with `cmake` and `ninja` on `PATH`:

```bat
cmake --preset windows-debug
cmake --build --preset windows-debug

cmake --preset windows-release
cmake --build --preset windows-release
```

The presets use `architecture.strategy = external`: Visual Studio supplies the compiler environment. In a terminal, initialize that environment through the VS developer prompt first.

### Prefer a solution file?

Generate one in a separate build directory. The Visual Studio 2026 generator requires CMake 4.2+.

```bat
cmake -S . -B out/build/vs2026 -G "Visual Studio 18 2026" -A x64
cmake --build out/build/vs2026 --config Debug --target baller
```

For Visual Studio 2022, use `Visual Studio 17 2022` as the generator.

### Build output

```text
out/build/windows-debug/bin/Debug/
├── baller.exe
└── shaders/
    ├── triangle.vert.spv
    └── triangle.frag.spv
```

Release outputs go to `out/build/windows-release/bin/Release/`. Solution builds use `out/build/vs2026/bin/<Config>/`.

## Shaders

One Slang source. Two entry points. Two SPIR-V modules.

| Source | Entry point | Stage | Output |
|---|---|---|---|
| `shaders/triangle.slang` | `vertexMain` | Vertex | `triangle.vert.spv` |
| `shaders/triangle.slang` | `fragmentMain` | Fragment | `triangle.frag.spv` |

The build targets **SPIR-V 1.6** and preserves entry point names with `-fvk-use-entrypoint-name`. Set the pipeline shader stage's `pName` to `vertexMain` or `fragmentMain` when connecting these modules.

- **Debug:** `-O0 -g`. **Other configurations:** `-O2`.
- **Matrix layout:** column-major, matching GLM's default storage. CPU/GPU structure alignment and multiplication order must still agree when resources are added.
- **Incremental builds:** Slang depfiles track source, include, and import dependencies.
- **Build failures:** shader errors fail the build.
- **Runtime:** Slang is used only at build time; Baller does not link its runtime library.

The current compiler emits the `DrawParameters` capability for `SV_VertexID`. Device initialization must query and enable `VkPhysicalDeviceVulkan11Features::shaderDrawParameters`. It must also check the physical device's Vulkan version and enable the rendering features it uses; instance creation alone does not do this.

### Add a shader

Register entry points in `CMakeLists.txt` before the `baller_shaders` target is created:

```cmake
baller_add_slang_shader(shaders/sprite.slang vertexMain vertex sprite.vert)
baller_add_slang_shader(shaders/sprite.slang fragmentMain fragment sprite.frag)
```

For a standalone Slang installation, override the compiler path:

```bat
cmake --preset windows-debug -DBALLER_SLANGC_EXECUTABLE=C:/tools/slang/bin/slangc.exe
```

## Roadmap

- [x] Windows foundation and Vulkan 1.4 instance setup.
- [x] Slang shader build pipeline.
- [ ] Device selection and graphics/present queues.
- [ ] Swapchain, command buffers, frame synchronization, and a clear color.
- [ ] First triangle through a Slang graphics pipeline.
- [ ] Vertex buffers, textured sprites, and an orthographic camera.
- [ ] Sprite batching, texture atlases, and safe GPU resource lifetimes.
- [ ] Scenes, input, animation, and a playable 2D demo.
- [ ] Profiling, benchmarks, and design notes.

A DX12 backend is a possible follow-up after the Vulkan demo is complete.

## References

- [Vulkan versions and porting guide](https://docs.vulkan.org/guide/latest/versions.html)
- [Slang compiler options](https://docs.shader-slang.org/en/stable/external/slang/docs/command-line-slangc-reference.html)
- [Visual Studio CMake Presets](https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs)
