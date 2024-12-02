#include "world_renderer.hpp"

#include "../lve_model.hpp"
#include "face_culling_chunk_mesher.hpp"
#include <cmath>
#include <iostream>
#include <limits>

WorldRenderer::WorldRenderer(lve::LveDevice& lveDevice, std::shared_ptr<World> world) {
    _world = world;
  _lastPosition = {
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max(),
      std::numeric_limits<float>::max()};
    /*
    std::cout << "World renderer" << std::endl;
    auto chunk = createChunk({0.0f, 0.0f, 0.0f});
    world->Chunks.emplace_back(chunk);
    FaceCullingChunkMesher chunkMesher;
    Mesh chunkMesh = chunkMesher.create(chunk);

    std::shared_ptr<lve::LveModel> chunkModel =
        std::make_shared<lve::LveModel>(lveDevice, chunkMesh);
    _models.emplace(chunk.Position, chunkModel);
    std::cout << "Adding model" << std::endl;
    */
}

void WorldRenderer::update(lve::LveDevice& lveDevice, const glm::vec3& playerPosition) {
    glm::vec3 position = round(playerPosition / 16.0f);
    
    if (position == _lastPosition) {
      return;
    }

    int r = 1;
    std::cout << "WorldRenderer::update() " << std::to_string(position.x) << ", "
              << std::to_string(position.z) << std::endl;
    //         *        
    // 1 2 3 4 5 6 7 8 9
    int a = position.x;
    int b = position.z;
    //( x - a )^2 + ( y - b )^2 = r^2
    double rSquared = pow(r, 2);
    std::vector<glm::vec3> validChunkPositions;
    for (int x = a - r; x <= a + r; x++) {
      //std::cout << "X" << std::to_string(x) << std::endl;
        for (int y = b - r; y <= b + r; y++) {
            double v = pow(x - a, 2) + pow(y - b, 2);
            if (v > rSquared) {
              //std::cout << "Outside of circle" << std::endl;
              continue;
            }


            glm::vec3 chunkPosition = {x, 0, y};
            validChunkPositions.push_back(chunkPosition);
            //std::cout << "Valid chunk position: " << std::to_string(chunkPosition.x) << ", "
            //          << std::to_string(chunkPosition.z) << std::endl;

            if (std::shared_ptr<World> world = _world.lock()) {
                std::vector<Chunk>::iterator it = find_if(
                    world->Chunks.begin(), 
                    world->Chunks.end(), 
                    [&chunkPosition](const Chunk &chunk) {return chunk.Position == chunkPosition; }
                );
                if (it != world->Chunks.end()) {
                    continue;
                }

                Chunk chunk = createChunk(chunkPosition);
                world->Chunks.emplace_back(chunk);
                // TODO: Create game object associated with chunk
                FaceCullingChunkMesher chunkMesher;
                Mesh chunkMesh = chunkMesher.create(chunk);
                
                std::shared_ptr<lve::LveModel> chunkModel = std::make_shared<lve::LveModel>(lveDevice, chunkMesh);
                /*
                auto chunkGameObject = lve::LveGameObject::createGameObject();
                chunkGameObject.model = chunkModel;
                chunkGameObject.transform.translation = chunk.Position * (float) chunk.Size;
                chunkGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
                _models.emplace(chunk.Position, std::move(chunkGameObject));
                */
                _models.emplace(chunk.Position, chunkModel);
            }
        }
    }

    
    if (std::shared_ptr<World> world = _world.lock()) {
        for (int i = world->Chunks.size() - 1; i >= 0; i--) {
            auto pos = world->Chunks[i].Position;
            std::vector<glm::vec3>::iterator validChunk = std::find_if(
                validChunkPositions.begin(), 
                validChunkPositions.end(), 
                [&pos](const glm::vec3& position) { return position == pos; }
            );
            if (validChunk != validChunkPositions.end()) {
                continue;
            }
            world->Chunks.erase(world->Chunks.begin() + i);
            _models.erase(pos);
        }
    }

    _lastPosition = position;
}

std::unordered_map<glm::vec3, std::shared_ptr<lve::LveModel>> WorldRenderer::getModels() const { 
    //std::cout << "Model Count: " << std::to_string(_models.size()) << std::endl;
    return _models;
}


Chunk WorldRenderer::createChunk(const glm::vec3& position) {
  srand(10);
  std::vector<uint32_t> chunkData;
  int chunkSize = 16;
  for (int i = 0; i < chunkSize; i++) {
    for (int j = 0; j < chunkSize; j++) {
      for (int k = 0; k < chunkSize; k++) {
        int x = i - chunkSize / 2;
        int y = j - chunkSize / 2;
        int z = k - chunkSize / 2;
        
        //uint32_t blockValue = rand() % 2;
        //chunkData.push_back(blockValue);
        chunkData.push_back(1);
      }
    }
  }

  Chunk chunk{position, chunkSize, chunkData};
  return chunk;
}