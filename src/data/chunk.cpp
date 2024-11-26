#include "chunk.hpp"

unsigned int Chunk::getBlock(const glm::vec3& position) const {
    return Data[position.x + position.y * Size + position.z * Size * Size];
};

void Chunk::setBlock(const glm::vec3& position, unsigned int value) {
    Data[position.x + position.y * Size + position.z * Size * Size] = value;
};