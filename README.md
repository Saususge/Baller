# Baller

### Simple. Fast. Vulkan-oriented 2D engine.

**C++20 / Vulkan 1.4 / Vulkan-Hpp RAII / Slang / Windows**

A solo engine project built from the first pixel up. Baller aims for a small, readable codebase, efficient sprite rendering, and a playable 2D demo backed by performance measurements.

**Status: early development.** Window setup, GPU selection, and logical device creation are in place. Shaders compile at build time; the first rendered triangle is the next milestone. Speed is a design goal, with benchmarks still ahead.

[Get started](#get-started) · [Shaders](#shaders) · [Roadmap](#roadmap)

---

## In the engine today

- Win32 window creation and event handling.
- Vulkan 1.4 instance setup, loader version checks, validation layers, and surface creation.
- GPU selection with Vulkan version, graphics/present queues, swapchain support, and feature checks.
- Logical device creation and graphics/present queue retrieval, including separate queue families.
- Vulkan-Hpp `vk::raii` ownership for the loader context, instance, debug messenger, surface, and device.
- Slang vertex and fragment shaders compiled to SPIR-V through CMake.
- Visual Studio presets for Windows x64 Debug and Release builds.

The swapchain and graphics pipeline are still to come. The current application opens a window and initializes a suitable GPU; it does not render the triangle yet. Selection uses the first GPU that meets all requirements.

## Get started

### Requirements

- Windows x64 and a GPU/driver that supports Vulkan 1.4.
- Visual Studio 2022 or newer. Development currently uses Visual Studio 2026.
- The **Desktop development with C++** workload, including:
  - MSVC x64/x86 build tools.
  - Windows 10 or Windows 11 SDK.
  - C++ CMake tools for Windows: CMake 3.24+ and Ninja.
- [Vulkan SDK](https://vulkan.lunarg.com/) 1.4+ with **Vulkan-Hpp headers** and the **Slang compiler** (`slangc.exe`).
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

## Vulkan ownership

Baller uses C++20 and `<vulkan/vulkan_raii.hpp>` through ordinary headers.
The Vulkan SDK supplies Vulkan-Hpp; no separate package or C++ module setup is needed.

- `VulkanInstance` owns a `vk::raii::Context`, `vk::raii::Instance`, and `vk::raii::DebugUtilsMessengerEXT`.
- `Platform::createSurface()` returns an owning `vk::raii::SurfaceKHR`.
- `VulkanDevice` stores the selected physical device, owns the logical device, and keeps graphics/present queue wrappers. Queue resources belong to the logical device.
- Queue requests are deduplicated when graphics and presentation use the same family; each requested family provides queue index zero.
- `getInstance()` returns a borrowed reference for creating child objects.
- Initialization errors propagate as exceptions to `main`, where they are logged.
- Resources are destroyed automatically in reverse declaration order: queue wrappers and logical device, surface, debug messenger, instance, context, then window.

Parents must outlive their children. RAII handles destruction, but does not wait for GPU work to finish. No work is submitted yet; GPU completion must be handled before resource destruction when rendering is added.
The validation callback retains Vulkan's C ABI, while object creation and ownership use Vulkan-Hpp.

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

The current compiler emits the `DrawParameters` capability for `SV_VertexID`. Device initialization checks and enables `vk::PhysicalDeviceVulkan11Features::shaderDrawParameters`, plus Vulkan 1.3 `dynamicRendering` and `synchronization2` for the upcoming renderer. It also requires a Vulkan 1.4 physical device, `VK_KHR_swapchain`, and nonempty surface format and present mode lists. The swapchain itself is not created yet.

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
- [x] Device selection and graphics/present queues.
- [ ] Swapchain, command buffers, frame synchronization, and a clear color.
- [ ] First triangle through a Slang graphics pipeline.
- [ ] Vertex buffers, textured sprites, and an orthographic camera.
- [ ] Sprite batching, texture atlases, and safe GPU resource lifetimes.
- [ ] Scenes, input, animation, and a playable 2D demo.
- [ ] Profiling, benchmarks, and design notes.

A DX12 backend is a possible follow-up after the Vulkan demo is complete.

## References

- [Vulkan-Hpp RAII programming guide](https://github.com/KhronosGroup/Vulkan-Hpp/blob/main/docs/VkRaiiProgrammingGuide.md)
- [Vulkan versions and porting guide](https://docs.vulkan.org/guide/latest/versions.html)
- [Slang compiler options](https://docs.shader-slang.org/en/stable/external/slang/docs/command-line-slangc-reference.html)
- [Visual Studio CMake Presets](https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs)
