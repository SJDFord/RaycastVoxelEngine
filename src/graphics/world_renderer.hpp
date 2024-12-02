#pragma once

#include <memory>
#include <vector>


#include <glm/glm.hpp>
#include "glm/gtx/hash.hpp"

#include "data/world.hpp";
#include "../lve_device.hpp"
#include "lve_game_object.hpp"

class WorldRenderer {
private:
    std::weak_ptr<World> _world;
    std::unordered_map<glm::vec3, std::shared_ptr<lve::LveModel>> _models;
    glm::vec3 _lastPosition;
    /*
    std::unordered_map<glm::vec3, unsigned int> _chunkGameObjects;
    std::vector<lve::LveGameObject> _gameObjects;
    */

    Chunk createChunk(const glm::vec3& position);
public:
    WorldRenderer(lve::LveDevice& lveDevice, std::shared_ptr<World> world);

    void update(lve::LveDevice& lveDevice, const glm::vec3& playerPosition);
    std::unordered_map<glm::vec3, std::shared_ptr<lve::LveModel>> getModels() const;
};