#include "face_culling_chunk_mesher.hpp"

#include "graphics_util.hpp"

#include <cstdlib>

Mesh FaceCullingChunkMesher::create(const Chunk& chunk) {
    unsigned int size = chunk.Size;
    std::vector<Vertex> vertices;
    unsigned int blockCount = 0;
    glm::vec3 chunkPosition = chunk.Position;
    glm::vec3 chunkFillColour = {1.0f, 1.0f, 1.0f};
    glm::vec3 chunkLineColor = {0.5f, 0.5f, 1.0f};
    for (int x = 0; x < size; x++) {
        for (int y = 0; y < size; y++) {
            for (int z = 0; z < size; z++) {
                glm::vec3 position = {x, y, z};
                uint32_t blockId = chunk.getBlock(position);
                bool isBlockInvisible = blockId < 1;
                
                if (isBlockInvisible) {
                    continue;
                }
                blockCount++;

                bool xEdge = x == 0 || x == size - 1;
                bool yEdge = y == 0 || y == size - 1;
                bool zEdge = z == 0 || z == size - 1;
                bool back = z == 0 || !isBlock(chunk, position + glm::vec3(0, 0, -1));
                bool front = z == size - 1 || !isBlock(chunk, position + glm::vec3(0, 0, 1));
                bool left = x == 0 || !isBlock(chunk, position + glm::vec3(-1, 0, 0));
                bool right = x == size - 1 || !isBlock(chunk, position + glm::vec3(1, 0, 0));
                bool top = y == size - 1 || !isBlock(chunk, position + glm::vec3(0, 1, 0));
                bool bottom = y == 0 || !isBlock(chunk, position + glm::vec3(0, -1, 0));
                // TODO: Improve this by pregenerating all 64 cube face combinations, put them in an array and use bit flags to access them
                /*
                glm::vec3 colour = {
                    float(rand()) / RAND_MAX,
                    float(rand()) / RAND_MAX,
                    float(rand()) / RAND_MAX};
                */
                bool isEdge = xEdge && yEdge && zEdge;

                glm::vec3 colour;
                switch (blockId) { 
                case 1: // Grass
                    colour = {0.11f, 0.49f, 0.122f};
                    break;
                case 2: // Sand
                    colour = {0.961f, 0.918f, 0.698f};
                    break;
                case 3: // Water
                    colour = {0.392f, 0.588f, 0.969f};
                    break;
                default:
                    colour = {1.0f, 0.0f, 0.0f};
                    break;
                }

                //glm::vec3 colour = isEdge ? chunkLineColor : chunkFillColour;
                Mesh cubeMesh =
                    createCubeMesh(position, colour, back, front, left, right, bottom, top);
                vertices.insert(vertices.end(), cubeMesh.Vertices.begin(), cubeMesh.Vertices.end());
            }
        }
    }
    unsigned int maxFaceCount = 6 * blockCount;
    unsigned int maxVertexCount = 4 * maxFaceCount;
    unsigned int vertexCount = vertices.size();
    unsigned int faceCount = vertexCount / 4; 
    float percentage = (1.0f-((float)vertexCount / (float)maxVertexCount)) * 100;
    /*
    fprintf(
        stdout, 
        "Generated %i faces (%i vertices) from a maximum of %i (%i) - (%.2f%% saving)\n", 
        faceCount,
        vertexCount,
        maxFaceCount,
        maxVertexCount, 
        percentage
    );
    */

    Mesh mesh = {vertices, {}};
	return mesh;
} 


bool FaceCullingChunkMesher::isBlock(const Chunk& chunk, const glm::vec3& position) {
    uint32_t blockID = chunk.getBlock(position);
    return blockID > 0;
}