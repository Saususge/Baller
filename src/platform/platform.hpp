#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <windows.h>

#include <vulkan/vulkan_raii.hpp>

namespace baller {

/// Compile-time flag for validation and debug diagnostics.
/// Enabled when NDEBUG is not defined.
#ifdef NDEBUG
inline constexpr bool kDebugBuild = false;
#else
inline constexpr bool kDebugBuild = true;
#endif

/// Native handles for the Win32 window and application instance.
struct WindowHandle {
  HWND hwnd = nullptr;
  HINSTANCE hinstance = nullptr;
};

/// Window creation settings.
struct WindowConfig {
  std::wstring title = L"baller";
  uint32_t width = 1280;
  uint32_t height = 960;
};

/// Window, event, and Vulkan surface interface.
class Platform {
public:
  virtual ~Platform() = default;

  /// Create a window using the supplied settings.
  virtual bool createWindow(const WindowConfig &config) = 0;

  /// Process pending events. Return false when a quit request is received.
  virtual bool pollEvents() = 0;

  /// Return the native window handles.
  virtual WindowHandle getWindowHandle() const = 0;

  /// Return the current framebuffer dimensions.
  virtual void getFramebufferSize(uint32_t &width, uint32_t &height) const = 0;

  /// Destroy the window.
  virtual void destroyWindow() = 0;

  /// Return the instance extensions required for surface creation.
  virtual std::vector<const char *> getRequiredInstanceExtensions() const = 0;

  /// Return an owning surface. Destroy it before the instance and window.
  virtual vk::raii::SurfaceKHR
  createSurface(const vk::raii::Instance &instance) const = 0;

  /// Create the platform implementation.
  static std::unique_ptr<Platform> create();
};

} // namespace baller
