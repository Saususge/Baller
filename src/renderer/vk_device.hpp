#pragma once

#include "vk_queue_families.hpp"

namespace baller {

/// Selects a physical device and owns its logical device and queue wrappers.
class VulkanDevice {
public:
  VulkanDevice(const vk::raii::Instance &instance,
               const vk::raii::SurfaceKHR &surface);
  ~VulkanDevice() = default;

  VulkanDevice(const VulkanDevice &) = delete;
  VulkanDevice &operator=(const VulkanDevice &) = delete;
  VulkanDevice(VulkanDevice &&) = delete;
  VulkanDevice &operator=(VulkanDevice &&) = delete;

  const vk::raii::PhysicalDevice &getPhysicalDevice() const noexcept {
    return m_physicalDevice;
  }
  const vk::raii::Device &getDevice() const noexcept { return m_device; }
  const vk::raii::Queue &getGraphicsQueue() const noexcept {
    return m_graphicsQueue;
  }
  const vk::raii::Queue &getPresentQueue() const noexcept {
    return m_presentQueue;
  }
  const QueueFamilyIndices &getQueueFamilies() const noexcept {
    return m_queueFamilies;
  }

private:
  void pickPhysicalDevice(const vk::raii::Instance &instance,
                          const vk::raii::SurfaceKHR &surface);
  void createLogicalDevice();

  // Reverse destruction order keeps the device alive for its queue wrappers.
  // Callers must finish submitted GPU work before destroying this owner.
  vk::raii::PhysicalDevice m_physicalDevice{nullptr};
  QueueFamilyIndices m_queueFamilies;
  vk::raii::Device m_device{nullptr};
  vk::raii::Queue m_graphicsQueue{nullptr};
  vk::raii::Queue m_presentQueue{nullptr};
};

} // namespace baller
