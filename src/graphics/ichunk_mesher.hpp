#pragma once
#include <vector>
#include "../engine/mesh.hpp"
#include "../data/chunk.hpp"

class IChunkMesher {
 public:
  virtual engine::Mesh create(const Chunk& chunk) = 0;
};