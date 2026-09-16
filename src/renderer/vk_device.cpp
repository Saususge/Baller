#include "vk_device.hpp"

#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "vk_instance.hpp"

namespace baller {
namespace {

constexpr std::array<const char *, 1> kRequiredDeviceExtensions = {
    vk::KHRSwapchainExtensionName};

using DeviceFeatures = vk::StructureChain<vk::PhysicalDeviceFeatures2,
                                         vk::PhysicalDeviceVulkan11Features,
                                         vk::PhysicalDeviceVulkan13Features>;

bool supportsRequiredExtensions(const vk::raii::PhysicalDevice &device) {
  const auto available = device.enumerateDeviceExtensionProperties();
  return std::ranges::all_of(kRequiredDeviceExtensions, [&](const char *name) {
    return std::ranges::any_of(available, [&](const auto &extension) {
      return std::strcmp(name, extension.extensionName.data()) == 0;
    });
  });
}

} // namespace

VulkanDevice::VulkanDevice(const vk::raii::Instance &instance,
                           const vk::raii::SurfaceKHR &surface) {
  pickPhysicalDevice(instance, surface);
  createLogicalDevice();
}

void VulkanDevice::pickPhysicalDevice(const vk::raii::Instance &instance,
                                      const vk::raii::SurfaceKHR &surface) {
  auto physicalDevices = instance.enumeratePhysicalDevices();
  if (physicalDevices.empty()) {
    throw std::runtime_error("Failed to find GPUs with Vulkan support.");
  }

  // Select the first suitable GPU; device ranking can be added when needed.
  for (auto &candidate : physicalDevices) {
    const auto properties = candidate.getProperties();
    const auto version = properties.apiVersion;
    std::cout << "GPU: " << properties.deviceName.data() << '\n'
              << "Vulkan Version: " << VK_API_VERSION_MAJOR(version) << '.'
              << VK_API_VERSION_MINOR(version) << '.'
              << VK_API_VERSION_PATCH(version) << '\n';

    if (version < kRequiredVulkanVersion) {
      std::cout << "Skipped: Vulkan 1.4 is required.\n";
      continue;
    }

    const auto indices = findQueueFamilies(candidate, surface);
    if (!indices.isComplete()) {
      std::cout << "Skipped: graphics and present queue families are required.\n";
      continue;
    }
    if (!supportsRequiredExtensions(candidate)) {
      std::cout << "Skipped: VK_KHR_swapchain is required.\n";
      continue;
    }
    if (candidate.getSurfaceFormatsKHR(*surface).empty() ||
        candidate.getSurfacePresentModesKHR(*surface).empty()) {
      std::cout << "Skipped: no surface formats or present modes are available.\n";
      continue;
    }

    const auto features = candidate.getFeatures2<
        vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
        vk::PhysicalDeviceVulkan13Features>();
    const auto &features11 = features.get<vk::PhysicalDeviceVulkan11Features>();
    const auto &features13 = features.get<vk::PhysicalDeviceVulkan13Features>();
    if (!features11.shaderDrawParameters || !features13.dynamicRendering ||
        !features13.synchronization2) {
      std::cout << "Skipped: shaderDrawParameters, dynamicRendering, and "
                   "synchronization2 are required.\n";
      continue;
    }

    m_queueFamilies = indices;
    m_physicalDevice = std::move(candidate);
    std::cout << "Selected GPU: " << properties.deviceName.data() << '\n'
              << "Graphics queue family: " << *indices.graphics << '\n'
              << "Present queue family: " << *indices.present << '\n';
    return;
  }

  throw std::runtime_error(
      "No suitable GPU found. Baller requires Vulkan 1.4, graphics/present "
      "queues, VK_KHR_swapchain, usable surface formats/present modes, "
      "shaderDrawParameters, dynamicRendering, and synchronization2.");
}

void VulkanDevice::createLogicalDevice() {
  const auto graphicsFamily = m_queueFamilies.graphics.value();
  const auto presentFamily = m_queueFamilies.present.value();
  const float queuePriority = 1.0f;

  vk::DeviceQueueCreateInfo graphicsQueueInfo;
  graphicsQueueInfo.setQueueFamilyIndex(graphicsFamily)
      .setQueueCount(1)
      .setPQueuePriorities(&queuePriority);
  std::vector<vk::DeviceQueueCreateInfo> queueInfos{graphicsQueueInfo};

  // Request each family only once when graphics and presentation share it.
  if (presentFamily != graphicsFamily) {
    auto presentQueueInfo = graphicsQueueInfo;
    presentQueueInfo.setQueueFamilyIndex(presentFamily);
    queueInfos.push_back(presentQueueInfo);
  }

  // Querying support does not enable features. Request only the ones we use.
  DeviceFeatures enabledFeatures;
  enabledFeatures.get<vk::PhysicalDeviceVulkan11Features>()
      .setShaderDrawParameters(true);
  enabledFeatures.get<vk::PhysicalDeviceVulkan13Features>()
      .setDynamicRendering(true)
      .setSynchronization2(true);

  vk::DeviceCreateInfo createInfo;
  createInfo.setQueueCreateInfos(queueInfos)
      .setPEnabledExtensionNames(kRequiredDeviceExtensions)
      .setPNext(&enabledFeatures.get<vk::PhysicalDeviceFeatures2>());
  m_device = vk::raii::Device(m_physicalDevice, createInfo);

  // The final argument is the queue index within its family, not a family index.
  m_graphicsQueue = vk::raii::Queue(m_device, graphicsFamily, 0);
  m_presentQueue = vk::raii::Queue(m_device, presentFamily, 0);
  std::cout << "Logical device created. Graphics and present queues acquired.\n"
            << "Enabled features: shaderDrawParameters, dynamicRendering, "
               "synchronization2.\n";
}

} // namespace baller
