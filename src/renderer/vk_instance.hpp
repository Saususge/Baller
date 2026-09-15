#pragma once

#include <string>
#include <vector>
#include <vulkan/vulkan.h>


namespace baller {

inline constexpr uint32_t kRequiredVulkanVersion = VK_API_VERSION_1_4;

/// Owns a Vulkan instance and its validation messenger.
class VulkanInstance {
public:
  VulkanInstance() = default;
  ~VulkanInstance();

  // Non-copyable and non-movable: this object owns Vulkan handles.
  VulkanInstance(const VulkanInstance &) = delete;
  VulkanInstance &operator=(const VulkanInstance &) = delete;

  /// Create the Vulkan instance.
  /// @param appName Application name.
  /// @param requiredExtensions Required instance extensions, including platform surface extensions.
  /// @param enableValidation Whether to enable validation layers.
  bool create(const std::string &appName,
              const std::vector<const char *> &requiredExtensions,
              bool enableValidation = true);

  /// Release owned Vulkan resources.
  void destroy();

  VkInstance getInstance() const { return m_instance; }
  bool isValidationEnabled() const { return m_validationEnabled; }

private:
  /// Check whether all requested validation layers are available.
  bool checkValidationLayerSupport() const;

  /// Set up the debug messenger.
  void setupDebugMessenger();
  void destroyDebugMessenger();

  /// Forward Vulkan validation messages to the application log.
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
                void *pUserData);

  VkInstance m_instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT m_debugMessenger = VK_NULL_HANDLE;
  bool m_validationEnabled = false;

  static const std::vector<const char *> s_validationLayers;
};

} // namespace baller
