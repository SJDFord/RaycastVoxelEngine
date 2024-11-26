#pragma once
#include <vector>
#include "mesh.hpp"
#include "../data/chunk.hpp"

class IChunkMesher {
 public:
  virtual Mesh create(const Chunk& chunk) = 0;
};