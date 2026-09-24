#include "mesh.hpp"

namespace engine {
    Mesh createCubeMesh(glm::vec3 position, glm::vec3 colour, bool back, bool front, bool left, bool right, bool bottom, bool top) {
    Mesh mesh = {{}, {}};

    

    if (back) {
        std::vector<Vertex> backVertices = {
                {CubeCoords::BackBottomLeft + position, colour, CubeNormals::Back, TextureCoords::BottomRight},
                {CubeCoords::BackTopRight + position, colour, CubeNormals::Back, TextureCoords::TopLeft},
                {CubeCoords::BackBottomRight + position, colour, CubeNormals::Back, TextureCoords::BottomLeft},

                {CubeCoords::BackTopRight + position, colour, CubeNormals::Back, TextureCoords::TopLeft},
                {CubeCoords::BackBottomLeft + position, colour, CubeNormals::Back, TextureCoords::BottomRight},
                {CubeCoords::BackTopLeft + position, colour, CubeNormals::Back, TextureCoords::TopRight}
        };
        mesh.Vertices.insert(mesh.Vertices.end(), backVertices.begin(), backVertices.end());
    }

    if (front) {
        std::vector<Vertex> frontVertices = {
            {CubeCoords::FrontBottomLeft + position, colour, CubeNormals::Front, TextureCoords::BottomLeft},
            {CubeCoords::FrontBottomRight + position, colour, CubeNormals::Front, TextureCoords::BottomRight},
            {CubeCoords::FrontTopRight + position, colour, CubeNormals::Front, TextureCoords::TopRight},

            {CubeCoords::FrontTopRight + position, colour, CubeNormals::Front, TextureCoords::TopRight},
            {CubeCoords::FrontTopLeft + position, colour, CubeNormals::Front, TextureCoords::TopLeft},
            {CubeCoords::FrontBottomLeft + position, colour, CubeNormals::Front,TextureCoords::BottomLeft},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), frontVertices.begin(), frontVertices.end());
    }

    if (left) {
        std::vector<Vertex> leftVertices = {
            {CubeCoords::FrontTopLeft + position, colour, CubeNormals::Left, TextureCoords::TopRight},
            {CubeCoords::BackTopLeft + position,  colour, CubeNormals::Left, TextureCoords::TopLeft},
            {CubeCoords::BackBottomLeft + position,  colour, CubeNormals::Left, TextureCoords::BottomLeft},

            {CubeCoords::BackBottomLeft + position, colour, CubeNormals::Left, TextureCoords::BottomLeft},
            {CubeCoords::FrontBottomLeft + position, colour, CubeNormals::Left, TextureCoords::BottomRight},
            {CubeCoords::FrontTopLeft + position, colour, CubeNormals::Left, TextureCoords::TopRight},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), leftVertices.begin(), leftVertices.end());
    }

    if (right) {
        std::vector<Vertex> rightVertices = {
            {CubeCoords::FrontTopRight + position, colour, CubeNormals::Right, TextureCoords::TopLeft},
            {CubeCoords::BackBottomRight + position, colour, CubeNormals::Right, TextureCoords::BottomRight},
            {CubeCoords::BackTopRight + position, colour, CubeNormals::Right, TextureCoords::TopRight},

            {CubeCoords::BackBottomRight + position, colour, CubeNormals::Right, TextureCoords::BottomRight},
            {CubeCoords::FrontTopRight + position, colour, CubeNormals::Right, TextureCoords::TopLeft},
            {CubeCoords::FrontBottomRight + position, colour, CubeNormals::Right, TextureCoords::BottomLeft},
        };
        mesh.Vertices.insert(mesh.Vertices.end(), rightVertices.begin(), rightVertices.end());
    }

    if (bottom) {
        std::vector<Vertex> bottomVertices = {
            {CubeCoords::BackBottomLeft + position, colour, CubeNormals::Bottom, TextureCoords::BottomLeft},
            {CubeCoords::BackBottomRight + position, colour, CubeNormals::Bottom, TextureCoords::BottomRight},
            {CubeCoords::FrontBottomRight + position, colour, CubeNormals::Bottom, TextureCoords::TopRight},

            {CubeCoords::FrontBottomRight + position, colour, CubeNormals::Bottom, TextureCoords::TopRight},
            {CubeCoords::FrontBottomLeft + position, colour, CubeNormals::Bottom, TextureCoords::TopLeft},
            {CubeCoords::BackBottomLeft + position, colour, CubeNormals::Bottom,TextureCoords::BottomLeft},
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
            {CubeCoords::BackTopLeft + position, colour, CubeNormals::Top, textureCoords[index][0]},
            {CubeCoords::FrontTopRight + position, colour, CubeNormals::Top, textureCoords[index][1]},
            {CubeCoords::BackTopRight + position, colour, CubeNormals::Top, textureCoords[index][2]},

            {CubeCoords::FrontTopRight + position, colour, CubeNormals::Top, textureCoords[index][1]},
            {CubeCoords::BackTopLeft + position, colour, CubeNormals::Top, textureCoords[index][0]},
            {CubeCoords::FrontTopLeft + position, colour, CubeNormals::Top, textureCoords[index][3]},
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
}