#include "vk_queue_families.hpp"

namespace baller {

QueueFamilyIndices findQueueFamilies(const vk::raii::PhysicalDevice &device,
                                    const vk::raii::SurfaceKHR &surface) {
  QueueFamilyIndices indices;
  const auto families = device.getQueueFamilyProperties();

  for (uint32_t index = 0; index < families.size(); ++index) {
    const auto &family = families[index];
    if (family.queueCount == 0) {
      continue;
    }

    const bool supportsGraphics =
        static_cast<bool>(family.queueFlags & vk::QueueFlagBits::eGraphics);
    const bool supportsPresent = device.getSurfaceSupportKHR(index, *surface);

    // Prefer one family for both roles, even if separate candidates came first.
    if (supportsGraphics && supportsPresent) {
      indices.graphics = index;
      indices.present = index;
      return indices;
    }

    if (supportsGraphics && !indices.graphics.has_value()) {
      indices.graphics = index;
    }
    if (supportsPresent && !indices.present.has_value()) {
      indices.present = index;
    }
  }

  // Different graphics and presentation families are also valid.
  return indices;
}

} // namespace baller
