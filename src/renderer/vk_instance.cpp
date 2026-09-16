#include "vk_instance.hpp"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace baller {

const std::vector<const char *> VulkanInstance::s_validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

VulkanInstance::VulkanInstance(
    const std::string &appName,
    const std::vector<const char *> &requiredExtensions, bool enableValidation)
    : m_validationEnabled(enableValidation) {
  // A Vulkan 1.0 loader has no version-query entry point.
  const uint32_t loaderVersion =
      m_context.getDispatcher()->vkEnumerateInstanceVersion
          ? m_context.enumerateInstanceVersion()
          : VK_API_VERSION_1_0;
  if (loaderVersion < kRequiredVulkanVersion) {
    throw std::runtime_error(
        "baller requires a Vulkan 1.4 or newer loader. Current version: " +
        std::to_string(VK_API_VERSION_MAJOR(loaderVersion)) + "." +
        std::to_string(VK_API_VERSION_MINOR(loaderVersion)));
  }

  if (m_validationEnabled && !checkValidationLayerSupport()) {
    std::cerr << "Warning: validation layers are unavailable. Validation is disabled.\n";
    m_validationEnabled = false;
  }

  vk::ApplicationInfo appInfo;
  appInfo.setPApplicationName(appName.c_str())
      .setApplicationVersion(VK_MAKE_API_VERSION(0, 0, 1, 0))
      .setPEngineName("baller")
      .setEngineVersion(VK_MAKE_API_VERSION(0, 0, 1, 0))
      .setApiVersion(kRequiredVulkanVersion);

  std::vector<const char *> extensions(requiredExtensions);
  if (m_validationEnabled &&
      std::ranges::none_of(extensions, [](const char *extension) {
        return std::strcmp(extension, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0;
      })) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  const auto availableExtensions = m_context.enumerateInstanceExtensionProperties();
  std::cout << "Available Vulkan extensions (" << availableExtensions.size()
            << "):\n";
  for (const auto &extension : availableExtensions) {
    std::cout << "  " << extension.extensionName.data() << '\n';
  }

  vk::DebugUtilsMessengerCreateInfoEXT debugInfo;
  debugInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                               vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
      .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                      vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);
  // Hpp versions expose either a C or C++ callback typedef with the same ABI.
  debugInfo.pfnUserCallback =
      reinterpret_cast<decltype(debugInfo.pfnUserCallback)>(&debugCallback);

  vk::InstanceCreateInfo createInfo;
  createInfo.setPApplicationInfo(&appInfo).setPEnabledExtensionNames(extensions);
  if (m_validationEnabled) {
    createInfo.setPEnabledLayerNames(s_validationLayers).setPNext(&debugInfo);
  }

  m_instance = vk::raii::Instance(m_context, createInfo);
  std::cout << "Vulkan instance created.\n";

  if (m_validationEnabled) {
    // The pNext callback covers instance creation/destruction; this owned
    // messenger covers validation messages throughout the instance's lifetime.
    debugInfo.messageSeverity |= vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose;
    m_debugMessenger = vk::raii::DebugUtilsMessengerEXT(m_instance, debugInfo);
    std::cout << "Vulkan debug messenger enabled.\n";
  }
}

bool VulkanInstance::checkValidationLayerSupport() const {
  const auto availableLayers = m_context.enumerateInstanceLayerProperties();
  for (const char *layerName : s_validationLayers) {
    const bool found = std::ranges::any_of(
        availableLayers, [layerName](const vk::LayerProperties &layer) {
          return std::strcmp(layerName, layer.layerName.data()) == 0;
        });
    if (!found) {
      std::cerr << "Validation layer unavailable: " << layerName << '\n';
      return false;
    }
  }
  return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT,
    const VkDebugUtilsMessengerCallbackDataEXT *callbackData, void *) noexcept {
  const char *prefix = "[VULKAN]";
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    prefix = "[VULKAN ERROR]";
  } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    prefix = "[VULKAN WARNING]";
  }

  // Exceptions must never cross the Vulkan callback boundary.
  std::fprintf(stderr, "%s %s\n", prefix, callbackData->pMessage);
  return VK_FALSE;
}

} // namespace baller
