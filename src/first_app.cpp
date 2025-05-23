#include "first_app.hpp"

#include "keyboard_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "data/chunk.hpp"
#include "data/world.hpp"
#include "graphics/face_culling_chunk_mesher.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace lve {

FirstApp::FirstApp() {
    
    
   globalPool =
      LveDescriptorPool::Builder(lveDevice)
          .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();
  _world = std::make_shared<World>();
  _worldRenderer = std::make_unique<WorldRenderer>(lveDevice, _world);


  
  loadGameObjects();
}

FirstApp::~FirstApp() {}

void FirstApp::run() {
  /*
  std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  auto globalSetLayout =
      LveDescriptorSetLayout::Builder(lveDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simpleRenderSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  KeyboardMovementController cameraController{};

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
    _worldRenderer->update(lveDevice, viewerObject.transform.translation);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();
      std::unordered_map<glm::vec3, std::shared_ptr<lve::LveModel>> models = _worldRenderer->getModels();
      LveGameObject::Map chunkGameObjects;
      for (auto positionModel : models) {
        //std::cout << "Processing game object" << std::endl;
        glm::vec3 position = positionModel.first;
        std::shared_ptr<lve::LveModel> model = positionModel.second;
        // gameObjects.emplace()

        auto chunkGameObject = LveGameObject::createGameObject();
        chunkGameObject.model = model;
        chunkGameObject.transform.translation = position * (float)16;
        chunkGameObject.transform.scale = {1.0f, 1.0f, 1.0f};
        //gameObjects.emplace(chunkGameObject.getId(), std::move(chunkGameObject));
        chunkGameObjects.emplace(chunkGameObject.getId(), std::move(chunkGameObject)); 
      }

      auto mainLight = LveGameObject::makePointLight(10.0f);
      mainLight.color = {1.0f, 1.0f, 1.0f};

      glm::vec3 lightOffset = {0.0f, 2.0f, 0.0f};
      mainLight.transform.translation = viewerObject.transform.translation + lightOffset;
      mainLight.transform.scale = {0.1f, 0.1f, 0.1f};
      chunkGameObjects.emplace(mainLight.getId(), std::move(mainLight));

      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          chunkGameObjects};

      // update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.inverseView = camera.getInverseView();
      pointLightSystem.update(frameInfo, ubo);
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      lveRenderer.beginSwapChainRenderPass(commandBuffer);

      // order here matters
      simpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
  */

  std::vector<std::unique_ptr<LveBuffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<LveBuffer>(
        lveDevice,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    uboBuffers[i]->map();
  }

  auto globalSetLayout =
      LveDescriptorSetLayout::Builder(lveDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  SimpleRenderSystem simpleRenderSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  PointLightSystem pointLightSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(),
      globalSetLayout->getDescriptorSetLayout()};
  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  KeyboardMovementController cameraController{};

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.moveInPlaneXZ(lveWindow.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();
      FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          gameObjects};

      // update
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.inverseView = camera.getInverseView();
      pointLightSystem.update(frameInfo, ubo);
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render
      lveRenderer.beginSwapChainRenderPass(commandBuffer);

      // order here matters
      simpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      lveRenderer.endSwapChainRenderPass(commandBuffer);
      lveRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(lveDevice.device());
}

void FirstApp::loadGameObjects() {

  /*
  _worldRenderer->update(lveDevice, {0, 0, 0});
  auto gameObjectList = _worldRenderer->getModels();
  for (auto gameObject : gameObjectList) {
    std::cout << "Processing game object" << std::endl;
    glm::vec3 position = gameObject.first;
    std::shared_ptr<lve::LveModel> model = gameObject.second;
    // gameObjects.emplace()

    auto chunkGameObject = LveGameObject::createGameObject();
    chunkGameObject.model = model;
    chunkGameObject.transform.translation = position * (float)16;
    chunkGameObject.transform.scale = {1.0f, 1.0f, 1.0f};
    gameObjects.emplace(chunkGameObject.getId(), std::move(chunkGameObject));
  }

  auto mainLight = LveGameObject::makePointLight(10.0f);
  mainLight.color = {1.0f, 1.0f, 1.0f};

  mainLight.transform.translation = {0.0f, -5.0f, 0.0f};
  mainLight.transform.scale = {1.0f, 1.0f, 1.0f};
  gameObjects.emplace(mainLight.getId(), std::move(mainLight));
  */

  /*
  srand(10);
  std::vector<uint32_t> chunkData;
  int chunkSize = 16;
  for (int i = 0; i < chunkSize; i++) {
    for (int j = 0; j < chunkSize; j++) {
      for (int k = 0; k < chunkSize; k++) {
        int x = i - chunkSize / 2;
        int y = j - chunkSize / 2;
        int z = k - chunkSize / 2;
        // unsigned int distance = pow(x, 2) + pow(y, 2) + pow(z, 2);
        // unsigned int blockValue= distance < 64 ? 1 : 0;  //rand() % 2;
        uint32_t blockValue = rand() % 2;
        chunkData.push_back(blockValue);
      }
    }
  }

  Chunk chunk1{{0.0f, 0.0f, 0.0f}, chunkSize, chunkData};
  Chunk chunk2{{0.2f, 0.0f, 0.0f}, chunkSize, chunkData};
  World world{{chunk1, chunk2}};

  for (const Chunk& chunk : world.Chunks) {
    FaceCullingChunkMesher chunkMesher;
    Mesh chunkMesh = chunkMesher.create(chunk);
    std::shared_ptr<LveModel> chunkModel = std::make_shared<LveModel>(lveDevice, chunkMesh);
    auto chunkGameObject = LveGameObject::createGameObject();
    chunkGameObject.model = chunkModel;
    chunkGameObject.transform.translation = chunk.Position * (float) chunk.Size;
    chunkGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
    gameObjects.emplace(chunkGameObject.getId(), std::move(chunkGameObject));
  }

  auto mainLight = LveGameObject::makePointLight(10.0f);
  mainLight.color = {
      1.0f,
      1.0f,
      1.0f
  };

  mainLight.transform.translation = {0.0f, -5.0f, 0.0f};
  mainLight.transform.scale = {1.0f, 1.0f, 1.0f};
  gameObjects.emplace(mainLight.getId(), std::move(mainLight));
  */
  std::shared_ptr<LveModel> lveModel =
      LveModel::createModelFromFile(lveDevice, "models/flat_vase.obj");
  auto flatVase = LveGameObject::createGameObject();
  flatVase.model = lveModel;
  flatVase.transform.translation = {-.5f, .5f, 0.f};
  flatVase.transform.scale = {3.f, 1.5f, 3.f};
  gameObjects.emplace(flatVase.getId(), std::move(flatVase));

  lveModel = LveModel::createModelFromFile(lveDevice, "models/smooth_vase.obj");
  auto smoothVase = LveGameObject::createGameObject();
  smoothVase.model = lveModel;
  smoothVase.transform.translation = {.5f, .5f, 0.f};
  smoothVase.transform.scale = {3.f, 1.5f, 3.f};
  gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

  lveModel = LveModel::createModelFromFile(lveDevice, "models/quad.obj");
  auto floor = LveGameObject::createGameObject();
  floor.model = lveModel;
  floor.transform.translation = {0.f, .5f, 0.f};
  floor.transform.scale = {3.f, 1.f, 3.f};
  gameObjects.emplace(floor.getId(), std::move(floor));

  std::vector<glm::vec3> lightColors{
      {1.f, .1f, .1f},
      {.1f, .1f, 1.f},
      {.1f, 1.f, .1f},
      {1.f, 1.f, .1f},
      {.1f, 1.f, 1.f},
      {1.f, 1.f, 1.f} 
  };

  for (int i = 0; i < lightColors.size(); i++) {
    auto pointLight = LveGameObject::makePointLight(0.2f);
    pointLight.color = lightColors[i];
    auto rotateLight = glm::rotate(
        glm::mat4(1.f),
        (i * glm::two_pi<float>()) / lightColors.size(),
        {0.f, -1.f, 0.f});
    pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
}

}  // namespace lve
