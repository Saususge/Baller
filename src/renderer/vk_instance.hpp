#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace baller {

inline constexpr uint32_t kRequiredVulkanVersion = VK_API_VERSION_1_4;

/// Owns the Vulkan loader context, instance, and validation messenger.
class VulkanInstance {
public:
  /// Create the instance, or throw if initialization fails.
  VulkanInstance(const std::string &appName,
                 const std::vector<const char *> &requiredExtensions,
                 bool enableValidation = true);
  ~VulkanInstance() = default;

  // Keep the owner stable while surfaces and devices borrow its instance.
  VulkanInstance(const VulkanInstance &) = delete;
  VulkanInstance &operator=(const VulkanInstance &) = delete;
  VulkanInstance(VulkanInstance &&) = delete;
  VulkanInstance &operator=(VulkanInstance &&) = delete;

  /// Borrow the instance. All child objects must be destroyed before this owner.
  const vk::raii::Instance &getInstance() const noexcept { return m_instance; }
  bool isValidationEnabled() const noexcept { return m_validationEnabled; }

private:
  bool checkValidationLayerSupport() const;

  /// Vulkan invokes this callback through its C ABI.
  static VKAPI_ATTR VkBool32 VKAPI_CALL
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT messageType,
                const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
                void *userData) noexcept;

  // Members are destroyed in reverse declaration order. The loader outlives
  // the instance, and the instance outlives its debug messenger.
  vk::raii::Context m_context;
  vk::raii::Instance m_instance{nullptr};
  vk::raii::DebugUtilsMessengerEXT m_debugMessenger{nullptr};
  bool m_validationEnabled = false;

  static const std::vector<const char *> s_validationLayers;
};

} // namespace baller
