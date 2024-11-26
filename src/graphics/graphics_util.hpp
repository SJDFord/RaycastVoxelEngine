#pragma once
#include <glm/glm.hpp>
#include "mesh.hpp"
//#include <engine/Model.h>

/*
std::shared_ptr<MeshBuffer> createMeshBuffer(const Mesh& mesh);
std::vector<std::shared_ptr<MeshBuffer>> createMeshBuffers(const std::vector<Mesh>& meshes);
std::vector<std::shared_ptr<MeshBuffer>> createMeshBuffers(const Model& model);
*/

struct TextureCoords {
    static constexpr glm::vec2 BottomLeft = glm::vec2(0.0f, 0.0f);
    static constexpr glm::vec2 TopLeft = glm::vec2(0.0f, 1.0f);
    static constexpr glm::vec2 BottomRight = glm::vec2(1.0f, 0.0f);
    static constexpr glm::vec2 TopRight = glm::vec2(1.0f, 1.0f);
};

struct CubeCoords {
    static constexpr glm::vec3 FrontBottomLeft = glm::vec3(-0.5f, -0.5f, 0.5f);
    static constexpr glm::vec3 FrontTopLeft = glm::vec3(-0.5f, 0.5f, 0.5f);
    static constexpr glm::vec3 FrontBottomRight = glm::vec3(0.5f, -0.5f, 0.5f);
    static constexpr glm::vec3 FrontTopRight = glm::vec3(0.5f, 0.5f, 0.5f);
    
    static constexpr glm::vec3 BackBottomLeft = glm::vec3(-0.5f, -0.5f, -0.5f);
    static constexpr glm::vec3 BackTopLeft = glm::vec3(-0.5f, 0.5f, -0.5f);
    static constexpr glm::vec3 BackBottomRight = glm::vec3(0.5f, -0.5f, -0.5f);
    static constexpr glm::vec3 BackTopRight = glm::vec3(0.5f, 0.5f, -0.5f);
};

struct CubeNormals {

    static constexpr glm::vec3 Back = glm::vec3(0.0f, 0.0f, -1.0f);
    static constexpr glm::vec3 Front = glm::vec3(0.0f, 0.0f, 1.0f);
    static constexpr glm::vec3 Left = glm::vec3(-1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
    static constexpr glm::vec3 Bottom = glm::vec3(0.0f, -1.0f, 0.0f);
    static constexpr glm::vec3 Top = glm::vec3(0.0f, 1.0f, 0.0f);
};

struct Meshes {
    static Mesh Cube;
    static Mesh Skybox;
};

Mesh createCubeMesh(glm::vec3 position, bool back, bool front, bool left, bool right, bool bottom, bool top);
/*
Mesh createCubeMeshOptimised(glm::vec3 position, bool back, bool front, bool left, bool right, bool bottom, bool top);
*/

Vertex createVertex(glm::vec3 position);