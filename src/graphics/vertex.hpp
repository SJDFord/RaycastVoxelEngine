#pragma once
#include "lve_device.hpp"
#include "lve_utils.hpp"

#include <glm/glm.hpp>
#include <vector>

struct Vertex {
  glm::vec3 position{};
  glm::vec3 color{};
  glm::vec3 normal{};
  glm::vec2 uv{};

  static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
  static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

  bool operator==(const Vertex &other) const;
};

namespace std {
    template <>
    struct hash<Vertex> {
    size_t operator()(Vertex const &vertex) const;
    };
}  // namespace std