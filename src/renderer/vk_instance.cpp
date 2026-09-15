#include "vk_instance.hpp"
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace baller {

// Default validation layers.
const std::vector<const char *> VulkanInstance::s_validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

VulkanInstance::~VulkanInstance() { destroy(); }

bool VulkanInstance::create(const std::string &appName,
                            const std::vector<const char *> &requiredExtensions,
                            bool enableValidation) {
  m_validationEnabled = enableValidation;

  // SDK/header support does not imply that the installed loader supports 1.4.
  uint32_t loaderVersion = VK_API_VERSION_1_0;
  auto enumerateInstanceVersion = reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
      vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkEnumerateInstanceVersion"));
  if (enumerateInstanceVersion != nullptr &&
      enumerateInstanceVersion(&loaderVersion) != VK_SUCCESS) {
    std::cerr << "Failed to query the Vulkan loader version." << std::endl;
    return false;
  }
  if (loaderVersion < kRequiredVulkanVersion) {
    std::cerr << "baller requires a Vulkan 1.4 or newer loader. "
              << "Check your graphics driver. Current version: "
              << VK_API_VERSION_MAJOR(loaderVersion) << "."
              << VK_API_VERSION_MINOR(loaderVersion) << std::endl;
    return false;
  }

  // Check validation layer availability.
  if (m_validationEnabled && !checkValidationLayerSupport()) {
    std::cerr
        << "Warning: validation layers are unavailable. Validation is disabled."
        << std::endl;
    m_validationEnabled = false;
  }

  // Application and engine metadata.
  VkApplicationInfo appInfo = {};
  appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  appInfo.pApplicationName = appName.c_str();
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
  appInfo.pEngineName = "baller";
  appInfo.engineVersion = VK_MAKE_API_VERSION(0, 0, 1, 0);
  appInfo.apiVersion = kRequiredVulkanVersion;

  // Collect the required instance extensions.
  std::vector<const char *> extensions(requiredExtensions);
  if (m_validationEnabled) {
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  }

  // List available instance extensions for diagnostics.
  uint32_t extensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
  std::vector<VkExtensionProperties> availableExtensions(extensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                         availableExtensions.data());

  std::cout << "Available Vulkan extensions (" << extensionCount
            << "):" << std::endl;
  for (const auto &ext : availableExtensions) {
    std::cout << "  " << ext.extensionName << std::endl;
  }

  // Configure instance creation.
  VkInstanceCreateInfo createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  createInfo.pApplicationInfo = &appInfo;
  createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
  createInfo.ppEnabledExtensionNames = extensions.data();

  // Capture validation messages during instance creation and destruction.
  VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {};
  if (m_validationEnabled) {
    createInfo.enabledLayerCount =
        static_cast<uint32_t>(s_validationLayers.size());
    createInfo.ppEnabledLayerNames = s_validationLayers.data();

    debugCreateInfo.sType =
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    debugCreateInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    debugCreateInfo.messageType =
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    debugCreateInfo.pfnUserCallback = debugCallback;

    createInfo.pNext = &debugCreateInfo;
  } else {
    createInfo.enabledLayerCount = 0;
    createInfo.pNext = nullptr;
  }

  // Create the Vulkan instance.
  VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
  if (result != VK_SUCCESS) {
    std::cerr << "Failed to create VkInstance. Error code: " << result << std::endl;
    return false;
  }

  std::cout << "VkInstance created." << std::endl;

  // Set up the debug messenger.
  if (m_validationEnabled) {
    setupDebugMessenger();
  }

  return true;
}

void VulkanInstance::destroy() {
  if (m_instance != VK_NULL_HANDLE) {
    destroyDebugMessenger();
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
    std::cout << "VkInstance destroyed." << std::endl;
  }
}

bool VulkanInstance::checkValidationLayerSupport() const {
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (const char *layerName : s_validationLayers) {
    bool found = false;
    for (const auto &layerProps : availableLayers) {
      if (std::strcmp(layerName, layerProps.layerName) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      std::cerr << "Validation layer unavailable: " << layerName << std::endl;
      return false;
    }
  }
  return true;
}

void VulkanInstance::setupDebugMessenger() {
  VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
  createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
  createInfo.pfnUserCallback = debugCallback;
  createInfo.pUserData = nullptr;

  // Load the extension entry point through the instance.
  auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
      vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

  if (func != nullptr) {
    VkResult result = func(m_instance, &createInfo, nullptr, &m_debugMessenger);
    if (result == VK_SUCCESS) {
      std::cout << "Vulkan debug messenger enabled." << std::endl;
    } else {
      std::cerr << "Failed to create the debug messenger." << std::endl;
    }
  } else {
    std::cerr << "Could not load vkCreateDebugUtilsMessengerEXT."
              << std::endl;
  }
}

void VulkanInstance::destroyDebugMessenger() {
  if (m_debugMessenger != VK_NULL_HANDLE) {
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr) {
      func(m_instance, m_debugMessenger, nullptr);
    }
    m_debugMessenger = VK_NULL_HANDLE;
  }
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanInstance::debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData) {
  const char *prefix = "[VULKAN]";
  if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
    prefix = "[VULKAN ERROR]";
  } else if (messageSeverity &
             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
    prefix = "[VULKAN WARNING]";
  }

  std::cerr << prefix << " " << pCallbackData->pMessage << std::endl;

  return VK_FALSE;
}

} // namespace baller
