#include <iostream>
#include <memory>

#include "platform/platform.hpp"
#include "renderer/vk_instance.hpp"

int main() {
#ifdef _WIN32
  // Use UTF-8 for Windows console output.
  SetConsoleOutputCP(CP_UTF8);
#endif

  // =============================================
  // 1. Create the window.
  // =============================================
  auto platform = baller::Platform::create();

  baller::WindowConfig windowConfig;
  windowConfig.title = L"baller";
  windowConfig.width = 1280;
  windowConfig.height = 960;

  if (!platform->createWindow(windowConfig)) {
    std::cerr << "Failed to create the window." << std::endl;
    return -1;
  }

  // =============================================
  // 2. Create the Vulkan instance.
  // =============================================
  baller::VulkanInstance vulkanInstance;

  auto requiredExtensions = platform->getRequiredInstanceExtensions();

  constexpr bool enableValidation = baller::kDebugBuild;

  if (!vulkanInstance.create("baller", requiredExtensions, enableValidation)) {
    std::cerr << "Failed to create the Vulkan instance." << std::endl;
    return -1;
  }

  // =============================================
  // 3. Create the Vulkan surface.
  // =============================================
  VkSurfaceKHR surface = platform->createSurface(vulkanInstance.getInstance());
  if (surface == VK_NULL_HANDLE) {
    std::cerr << "Failed to create the Vulkan surface." << std::endl;
    return -1;
  }

  std::cout << "========================================" << std::endl;
  std::cout << "baller initialized." << std::endl;
  std::cout << "Press ESC to exit." << std::endl;
  std::cout << "========================================" << std::endl;

  // =============================================
  // 4. Run the main loop.
  // =============================================
  while (platform->pollEvents()) {
    // Frame rendering will be added here in the next milestone.
  }

  // =============================================
  // 5. Release resources in reverse creation order.
  // =============================================
  // Destroy the surface.
  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(vulkanInstance.getInstance(), surface, nullptr);
    std::cout << "Vulkan surface destroyed." << std::endl;
  }

  // VulkanInstance releases its resources in its destructor.
  // Platform destroys the window in its destructor.

  std::cout << "baller shutting down." << std::endl;
  return 0;
}
