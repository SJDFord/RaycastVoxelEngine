#include "face_culling_chunk_mesher.hpp"

#include "graphics_util.hpp"

Mesh FaceCullingChunkMesher::create(const Chunk& chunk) {
    unsigned int size = chunk.Size;
    std::vector<Vertex> vertices;
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            for (int z = 0; z < size; z++) {
                glm::vec3 position = {x, y, z};
                bool isBlockInvisible = !isBlock(chunk, position);
                
                if (isBlockInvisible) {
                    continue;
                }

                bool back = z == 0 || !isBlock(chunk, position + glm::vec3(0, 0, -1));
                bool front = z == size - 1 || !isBlock(chunk, position + glm::vec3(0, 0, 1));
                bool left = x == 0 || !isBlock(chunk, position + glm::vec3(-1, 0, 0));
                bool right = x == size - 1 || !isBlock(chunk, position + glm::vec3(1, 0, 0));
                bool top = y == size - 1 || !isBlock(chunk, position + glm::vec3(0, 1, 0));
                bool bottom = y == 0 || !isBlock(chunk, position + glm::vec3(0, -1, 0));
                // TODO: Improve this by pregenerating all 64 cube face combinations, put them in an array and use bit flags to access them
                Mesh cubeMesh = createCubeMesh(position, back, front, left, right, bottom, top);
                vertices.insert(vertices.end(), cubeMesh.Vertices.begin(), cubeMesh.Vertices.end());
            }
        }
    }
    fprintf(stdout, "Face culling vertices (%i)\n", vertices.size());

    Mesh mesh = {vertices, {}};
	return mesh;
} 


bool FaceCullingChunkMesher::isBlock(const Chunk& chunk, const glm::vec3& position) {
    uint32_t blockID = chunk.getBlock(position);
    return blockID > 0;
}