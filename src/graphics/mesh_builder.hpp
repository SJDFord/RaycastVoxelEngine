#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include "vertex.hpp"
#include "mesh.hpp"

struct MeshBuilder {
    Mesh loadMesh(const std::string &filepath);
};