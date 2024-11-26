#pragma once

#include <vector>
#include "vertex.hpp"

struct Mesh {
  std::vector<Vertex> Vertices;
  std::vector<uint32_t> Indices;
  // std::vector<std::shared_ptr<Texture>> Textures;
};