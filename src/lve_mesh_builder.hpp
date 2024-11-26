#pragma once
#include <vector>
#include <unordered_map>
#include <iostream>
#include "lve_vertex.hpp"
#include "lve_mesh.hpp"

namespace lve {
    struct Builder {
        Mesh loadMesh(const std::string &filepath);
    };
}