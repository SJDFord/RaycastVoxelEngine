#pragma once
#include "lve_device.hpp"
#include "lve_utils.hpp"

#include <glm/glm.hpp>
#include <vector>

namespace engine {

struct Vertex {
  glm::vec3 position{};
  glm::vec3 color{};
  glm::vec3 normal{};
  glm::vec2 uv{};

  static std::vector<vk::VertexInputBindingDescription> getBindingDescriptions();
  static std::vector<vk::VertexInputAttributeDescription> getAttributeDescriptions();

  bool operator==(const Vertex &other) const;
};

}

namespace std {
    template <>
    struct hash<engine::Vertex> {
    size_t operator()(engine::Vertex const &vertex) const;
    };
}  // namespace std

