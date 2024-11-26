#include "graphics_util.hpp"

#include <random>

/*
std::shared_ptr<MeshBuffer> createMeshBuffer(const Mesh& mesh) {
    return std::make_shared<MeshBuffer>(mesh);
}

std::vector<std::shared_ptr<MeshBuffer>> createMeshBuffers(const std::vector<Mesh>& meshes) {
    std::vector<std::shared_ptr<MeshBuffer>> meshBuffers;
    for (int i = 0; i < meshes.size(); i++) {
        Mesh m = meshes[i];
        meshBuffers.push_back(createMeshBuffer(m));
    }
    return meshBuffers;
}

std::vector<std::shared_ptr<MeshBuffer>> createMeshBuffers(const Model& model) {
    return createMeshBuffers(model.Meshes);
}
*/

Mesh Meshes::Cube = {
        {
            // Back
            {CubeCoords::BackBottomLeft, {}, CubeNormals::Back, TextureCoords::BottomRight},
        {CubeCoords::BackTopRight, {}, CubeNormals::Back, TextureCoords::TopLeft},
        {CubeCoords::BackBottomRight, {}, CubeNormals::Back, TextureCoords::BottomLeft},

            {CubeCoords::BackTopRight, {}, CubeNormals::Back, TextureCoords::TopLeft},
        {CubeCoords::BackBottomLeft, {}, CubeNormals::Back, TextureCoords::BottomRight},
        {CubeCoords::BackTopLeft, {}, CubeNormals::Back, TextureCoords::TopRight},
            

            // Front
        {CubeCoords::FrontBottomLeft, {}, CubeNormals::Front, TextureCoords::BottomLeft},
        {CubeCoords::FrontBottomRight, {}, CubeNormals::Front, TextureCoords::BottomRight},
        {CubeCoords::FrontTopRight, {}, CubeNormals::Front, TextureCoords::TopRight},

            {CubeCoords::FrontTopRight, {}, CubeNormals::Front, TextureCoords::TopRight},
        {CubeCoords::FrontTopLeft, {}, CubeNormals::Front, TextureCoords::TopLeft},
        {CubeCoords::FrontBottomLeft, {}, CubeNormals::Front, TextureCoords::BottomLeft},

            // Left
            {CubeCoords::FrontTopLeft, {}, CubeNormals::Left, TextureCoords::TopRight},
            {CubeCoords::BackTopLeft,  {}, CubeNormals::Left, TextureCoords::TopLeft},
            {CubeCoords::BackBottomLeft,  {}, CubeNormals::Left, TextureCoords::BottomLeft},

            {CubeCoords::BackBottomLeft, {}, CubeNormals::Left, TextureCoords::BottomLeft},
            {CubeCoords::FrontBottomLeft, {}, CubeNormals::Left, TextureCoords::BottomRight},
            {CubeCoords::FrontTopLeft, {}, CubeNormals::Left, TextureCoords::TopRight},

            // Right
            {CubeCoords::FrontTopRight, {}, CubeNormals::Right, TextureCoords::TopLeft},
            {CubeCoords::BackBottomRight, {}, CubeNormals::Right, TextureCoords::BottomRight},
            {CubeCoords::BackTopRight, {}, CubeNormals::Right, TextureCoords::TopRight},

            {CubeCoords::BackBottomRight, {}, CubeNormals::Right, TextureCoords::BottomRight},
            {CubeCoords::FrontTopRight, {}, CubeNormals::Right, TextureCoords::TopLeft},
            {CubeCoords::FrontBottomRight, {}, CubeNormals::Right, TextureCoords::BottomLeft},

            // Bottom
            {CubeCoords::BackBottomLeft, {}, CubeNormals::Bottom, TextureCoords::BottomLeft},
            {CubeCoords::BackBottomRight, {}, CubeNormals::Bottom, TextureCoords::BottomRight},
            {CubeCoords::FrontBottomRight, {}, CubeNormals::Bottom, TextureCoords::TopRight},

            {CubeCoords::FrontBottomRight, {}, CubeNormals::Bottom, TextureCoords::TopRight},
            {CubeCoords::FrontBottomLeft, {}, CubeNormals::Bottom, TextureCoords::TopLeft},
            {CubeCoords::BackBottomLeft, {}, CubeNormals::Bottom,TextureCoords::BottomLeft},

            // Top
            {CubeCoords::BackTopLeft, {}, CubeNormals::Top, TextureCoords::TopLeft},
            {CubeCoords::FrontTopRight, {}, CubeNormals::Top, TextureCoords::BottomRight},
            {CubeCoords::BackTopRight, {}, CubeNormals::Top, TextureCoords::TopRight},

            {CubeCoords::FrontTopRight, {}, CubeNormals::Top, TextureCoords::BottomRight},
            {CubeCoords::BackTopLeft, {}, CubeNormals::Top, TextureCoords::TopLeft},
            {CubeCoords::FrontTopLeft, {}, CubeNormals::Top, TextureCoords::BottomLeft},
        },
        {}
    };

Vertex createVertex(glm::vec3 position) {
    return { position, {}, {} };
}

Mesh Meshes::Skybox = {
    { 
        createVertex({-1.0f,  1.0f, -1.0f}),
        createVertex({-1.0f, -1.0f, -1.0f}),
        createVertex({ 1.0f, -1.0f, -1.0f}),
        createVertex({ 1.0f, -1.0f, -1.0f}),
        createVertex({ 1.0f,  1.0f, -1.0f}),
        createVertex({-1.0f,  1.0f, -1.0f}),

        createVertex({-1.0f, -1.0f,  1.0f}),
        createVertex({-1.0f, -1.0f, -1.0f}),
        createVertex({-1.0f,  1.0f, -1.0f}),
        createVertex({-1.0f,  1.0f, -1.0f}),
        createVertex({-1.0f,  1.0f,  1.0f}),
        createVertex({-1.0f, -1.0f,  1.0f}),
        
        createVertex({ 1.0f, -1.0f, -1.0f}),
        createVertex({ 1.0f, -1.0f,  1.0f}),
        createVertex({ 1.0f,  1.0f,  1.0f}),
        createVertex({ 1.0f,  1.0f,  1.0f}),
        createVertex({ 1.0f,  1.0f, -1.0f}),
        createVertex({ 1.0f, -1.0f, -1.0f}),
        
        createVertex({-1.0f, -1.0f,  1.0f}),
        createVertex({-1.0f,  1.0f,  1.0f}),
        createVertex({ 1.0f,  1.0f,  1.0f}),
        createVertex({ 1.0f,  1.0f,  1.0f}),
        createVertex({ 1.0f, -1.0f,  1.0f}),
        createVertex({-1.0f, -1.0f,  1.0f}),
        
        createVertex({-1.0f,  1.0f, -1.0f}),
        createVertex({ 1.0f,  1.0f, -1.0f}),
        createVertex({ 1.0f,  1.0f,  1.0f}),
        createVertex({ 1.0f,  1.0f,  1.0f}),
        createVertex({-1.0f,  1.0f,  1.0f}),
        createVertex({-1.0f,  1.0f, -1.0f}),
        
        createVertex({-1.0f, -1.0f, -1.0f}),
        createVertex({-1.0f, -1.0f,  1.0f}),
        createVertex({ 1.0f, -1.0f, -1.0f}),
        createVertex({ 1.0f, -1.0f, -1.0f}),
        createVertex({-1.0f, -1.0f,  1.0f}),
        createVertex({ 1.0f, -1.0f,  1.0f})
    }, // verts
    {}, // indices
    //{} // textures
};


Mesh createCubeMesh(glm::vec3 position, bool back, bool front, bool left, bool right, bool bottom, bool top) {
  Mesh mesh = {{}, {}};

    

    if (back) {
        std::vector<Vertex> backVertices = {
                {CubeCoords::BackBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Back, TextureCoords::BottomRight},
                {CubeCoords::BackTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Back, TextureCoords::TopLeft},
                {CubeCoords::BackBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Back, TextureCoords::BottomLeft},

                {CubeCoords::BackTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Back, TextureCoords::TopLeft},
                {CubeCoords::BackBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Back, TextureCoords::BottomRight},
                {CubeCoords::BackTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Back, TextureCoords::TopRight}
        };
        mesh.Vertices.insert(mesh.Vertices.end(), backVertices.begin(), backVertices.end());
    }

    if (front) {
        std::vector<Vertex> frontVertices = {
            {CubeCoords::FrontBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Front, TextureCoords::BottomLeft},
            {CubeCoords::FrontBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Front, TextureCoords::BottomRight},
            {CubeCoords::FrontTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Front, TextureCoords::TopRight},

            {CubeCoords::FrontTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Front, TextureCoords::TopRight},
            {CubeCoords::FrontTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Front, TextureCoords::TopLeft},
            {CubeCoords::FrontBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Front,TextureCoords::BottomLeft},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), frontVertices.begin(), frontVertices.end());
    }

    if (left) {
        std::vector<Vertex> leftVertices = {
            {CubeCoords::FrontTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Left, TextureCoords::TopRight},
            {CubeCoords::BackTopLeft + position,  {1.0f, 1.0f, 1.0f}, CubeNormals::Left, TextureCoords::TopLeft},
            {CubeCoords::BackBottomLeft + position,  {1.0f, 1.0f, 1.0f}, CubeNormals::Left, TextureCoords::BottomLeft},

            {CubeCoords::BackBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Left, TextureCoords::BottomLeft},
            {CubeCoords::FrontBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Left, TextureCoords::BottomRight},
            {CubeCoords::FrontTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Left, TextureCoords::TopRight},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), leftVertices.begin(), leftVertices.end());
    }

    if (right) {
        std::vector<Vertex> rightVertices = {
            {CubeCoords::FrontTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Right, TextureCoords::TopLeft},
            {CubeCoords::BackBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Right, TextureCoords::BottomRight},
            {CubeCoords::BackTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Right, TextureCoords::TopRight},

            {CubeCoords::BackBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Right, TextureCoords::BottomRight},
            {CubeCoords::FrontTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Right, TextureCoords::TopLeft},
            {CubeCoords::FrontBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Right, TextureCoords::BottomLeft},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), rightVertices.begin(), rightVertices.end());
    }

    if (bottom) {
        std::vector<Vertex> bottomVertices = {
            {CubeCoords::BackBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Bottom, TextureCoords::BottomLeft},
            {CubeCoords::BackBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Bottom, TextureCoords::BottomRight},
            {CubeCoords::FrontBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Bottom, TextureCoords::TopRight},

            {CubeCoords::FrontBottomRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Bottom, TextureCoords::TopRight},
            {CubeCoords::FrontBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Bottom, TextureCoords::TopLeft},
            {CubeCoords::BackBottomLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Bottom,TextureCoords::BottomLeft},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), bottomVertices.begin(), bottomVertices.end());
    }

    if (top) {
        //int index = rand() % 2;
        int index = 0;

        // 0   2
        // 3   1

        // 3   0
        // 1   2

        std::vector<std::vector<glm::vec2>> textureCoords = {
            {
                TextureCoords::TopLeft,
                TextureCoords::BottomRight,
                TextureCoords::TopRight,
                TextureCoords::BottomLeft
            },
            {
                TextureCoords::BottomLeft,
                TextureCoords::TopLeft,
                TextureCoords::BottomRight,
                TextureCoords::TopRight
            }
        };

        std::vector<Vertex> topVertices = {
            {CubeCoords::BackTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][0]},
            {CubeCoords::FrontTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][1]},
            {CubeCoords::BackTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][2]},

            {CubeCoords::FrontTopRight + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][1]},
            {CubeCoords::BackTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][0]},
            {CubeCoords::FrontTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][3]},
        };

        mesh.Vertices.insert(mesh.Vertices.end(), topVertices.begin(), topVertices.end());
        std::vector<unsigned int> indicies = {
            0, 1, 2,
            2, 0, 3
        };
        //mesh.Indices = indicies;
    }

    return mesh;
}

/*
Mesh createCubeMeshOptimised(glm::vec3 position, bool back, bool front, bool left, bool right, bool bottom, bool top) {
    std::vector<glm::vec3> vertices[8] =
    {
        { -1.0f, -1.0f, -1.0f },
        { 1.0f, -1.0f, -1.0f },
        { 1, 1, -1 },
        { -1, 1, -1 },
        { -1, -1, 1 },
        { 1, -1, 1 },
        { 1, 1, 1 },
        { -1, 1, 1 }
    };

    mesh.Vertices.insert(mesh.Vertices.end(), topVertices.begin(), topVertices.end());

    
    if (top) {
        //int index = rand() % 2;
        int index = 0;

        // 0   2
        // 3   1

        // 3   0
        // 1   2

        std::vector<std::vector<glm::vec2>> textureCoords = {
            {
                TextureCoords::TopLeft,
                TextureCoords::BottomRight,
                TextureCoords::TopRight,
                TextureCoords::BottomLeft
            },
            {
                TextureCoords::BottomLeft,
                TextureCoords::TopLeft,
                TextureCoords::BottomRight,
                TextureCoords::TopRight
            }
        };

        std::vector<Vertex> topVertices = {
            {CubeCoords::BackTopLeft + position, {1.0f, 1.0f, 1.0f}, CubeNormals::Top, textureCoords[index][0]},
            {CubeCoords::FrontTopRight + position, {}, CubeNormals::Top, textureCoords[index][1]},
            {CubeCoords::BackTopRight + position, {}, CubeNormals::Top, textureCoords[index][2]},

            {CubeCoords::FrontTopRight + position, {}, CubeNormals::Top, textureCoords[index][1]},
            {CubeCoords::BackTopLeft + position, {}, CubeNormals::Top, textureCoords[index][0]},
            {CubeCoords::FrontTopLeft + position, {}, CubeNormals::Top, textureCoords[index][3]},
        };

        mesh.Vertices.insert(mesh.Vertices.end(), topVertices.begin(), topVertices.end());
        std::vector<unsigned int> indicies = {
            0, 1, 2,
            2, 0, 3
        };
        //mesh.Indices = indicies;
    }

    return mesh;
}
*/