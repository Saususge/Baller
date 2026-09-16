#pragma once

#include <cstdint>
#include <optional>
#include <vulkan/vulkan_raii.hpp>

namespace baller {

struct QueueFamilyIndices {
  // Index zero is valid; an empty optional means no suitable family was found.
  std::optional<uint32_t> graphics;
  std::optional<uint32_t> present;

  bool isComplete() const noexcept {
    return graphics.has_value() && present.has_value();
  }
};

// Presentation support depends on the specific window surface.
QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice &device,
                                    const vk::raii::SurfaceKHR &surface);

} // namespace baller
