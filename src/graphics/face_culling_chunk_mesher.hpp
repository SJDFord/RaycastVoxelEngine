#pragma once

#include "ichunk_mesher.hpp"

class FaceCullingChunkMesher : public IChunkMesher {
public:
    engine::Mesh create(const Chunk& chunk);

private:
    bool isBlock(const Chunk& chunk, const glm::vec3& position);
};