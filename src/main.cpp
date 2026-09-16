#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <algorithm>

#include "platform/platform.hpp"
#include "renderer/vk_device.hpp"
#include "renderer/vk_instance.hpp"

vk::Extent2D chooseSwapExtent(vk::SurfaceCapabilitiesKHR const &capabilities,
                              const baller::Platform &platform)
{
    if (capabilities.currentExtent.width !=
        std::numeric_limits<uint32_t>::max())
        return capabilities.currentExtent;
    uint32_t width = 0;
    uint32_t height = 0;
    platform.getFramebufferSize(width, height);
    return vk::Extent2D{
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                            capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                            capabilities.maxImageExtent.height)
    };
}

// Creating swapchain
void createSwapChain(const vk::raii::PhysicalDevice &physicalDevice,
                     const vk::raii::SurfaceKHR &surface,
                     const baller::Platform &platform)
{
    const auto surfaceCapabilities =
        physicalDevice.getSurfaceCapabilitiesKHR(*surface);
    vk::Extent2D swapChainExtent =
        chooseSwapExtent(surfaceCapabilities, platform);
}

int main() {
	// Use UTF-8 for Windows console output.
	SetConsoleOutputCP(CP_UTF8);

	try {
		// Declare parents first so child resources are destroyed before them.
		auto platform = baller::Platform::create();
		baller::WindowConfig windowConfig;
		windowConfig.width = 1280;
		windowConfig.height = 960;
		if (!platform->createWindow(windowConfig)) {
			std::cerr << "Failed to create the window.\n";
			return EXIT_FAILURE;
		}

		baller::VulkanInstance vulkanInstance( "baller", platform->getRequiredInstanceExtensions(), baller::kDebugBuild);
		auto surface = platform->createSurface(vulkanInstance.getInstance());
		baller::VulkanDevice vulkanDevice(vulkanInstance.getInstance(), surface);

		std::cout << "baller initialized.\nPress ESC to exit.\n";
        createSwapChain(vulkanDevice.getPhysicalDevice(), surface, *platform);
		while (platform->pollEvents())
		{
			// Frame rendering will be added here in the next milestone.
		}

		// No GPU work is submitted yet. Rendering will require waiting before cleanup.
		// Automatic cleanup: device, surface, debug messenger, instance, context, window.
		std::cout << "baller shutting down.\n";
	}
	catch (const std::exception& error)
	{
		// Stack unwinding releases any resources created before the failure.
		std::cerr << "Failed to initialize or run baller: " << error.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
