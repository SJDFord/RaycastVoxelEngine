#pragma once
#include <vector>
#include <glm/glm.hpp>

struct Chunk {
    uint32_t Size; // Cube side length
    std::vector<uint32_t> Data; // Data inside of chunk

    unsigned int getBlock(const glm::vec3& position) const;
    void setBlock(const glm::vec3& position, unsigned int value);
};