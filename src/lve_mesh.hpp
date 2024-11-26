#pragma once

#include <vector>
#include "lve_vertex.hpp"

namespace lve {
struct Mesh {
  std::vector<Vertex> Vertices;
  std::vector<uint32_t> Indices;
  // std::vector<std::shared_ptr<Texture>> Textures;
};
}  // namespace lve