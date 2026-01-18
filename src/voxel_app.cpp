#include "data/chunk.hpp"
#include "data/world.hpp"
#include "voxel_app.hpp"
#include "graphics/face_culling_chunk_mesher.hpp"
#include "fps_movement_controller.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "graphics/graphics_util.hpp"

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
#include <keyboard_movement_controller.hpp>
#include <image.hpp>

namespace lve {

VoxelApp::VoxelApp() {
  globalPool =
      LveDescriptorPool::Builder(lveDevice)
          .setMaxSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, LveSwapChain::MAX_FRAMES_IN_FLIGHT)
          .build();
  _world = std::make_shared<World>();
  _worldRenderer = std::make_unique<WorldRenderer>(lveDevice, _world);

  loadGameObjects();
}

VoxelApp::~VoxelApp() {}

void VoxelApp::run() {
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
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::unique_ptr<Image> image = std::make_unique<Image>(lveDevice, "textures/jungle-brick-with-moss.png");
  VkDescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  imageInfo.imageView = image->getImageView();
  imageInfo.sampler = image->getSampler();

  std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    LveDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imageInfo)
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
  FpsMovementController cameraController{lveWindow.getGLFWwindow()};

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!lveWindow.shouldClose()) {
    glfwPollEvents();

    if (glfwGetKey(lveWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
      lveWindow.close();
    }

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.updateView(lveWindow.getGLFWwindow(), frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();
      
      std::unordered_map<glm::vec3, std::shared_ptr<lve::LveModel>> models = _worldRenderer->getModels(); 

      auto mainLight = LveGameObject::makePointLight(10.0f);
      mainLight.color = {1.0f, 1.0f, 1.0f};

      glm::vec3 lightOffset = {0.0f, 2.0f, 0.0f};
      mainLight.transform.translation = viewerObject.transform.translation + lightOffset;
      mainLight.transform.scale = {0.1f, 0.1f, 0.1f};

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

  
  lveDevice.waitIdle();
}

void VoxelApp::loadGameObjects() {
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

  auto testGameObject = LveGameObject::createGameObject();
  glm::vec3 position = {1.0f, 1.0f, 1.0f};
  auto testMesh = createCubeMesh(position, {0.0f, 0.5f, 0.5f}, true, true, true, true, true, true);
  std::shared_ptr<LveModel> testModel = std::make_shared<LveModel>(lveDevice, testMesh);
  testGameObject.model = testModel;
  testGameObject.transform.translation = position;  // chunk.Position * (float)chunk.Size;
  testGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
  gameObjects.emplace(testGameObject.getId(), std::move(testGameObject));

  auto testGameObject2 = LveGameObject::createGameObject();
  glm::vec3 position2 = {2.0f, 1.0f, 1.0f};
  auto testMesh2 = createCubeMesh(position2, {1.0f, 0.65f, 0.0f}, true, true, true, true, true, true);
  std::shared_ptr<LveModel> testModel2 = std::make_shared<LveModel>(lveDevice, testMesh2);
  testGameObject2.model = testModel2;
  testGameObject2.transform.translation = position2;  // chunk.Position * (float)chunk.Size;
  testGameObject2.transform.scale = {0.2f, 0.2f, 0.2f};
  gameObjects.emplace(testGameObject2.getId(), std::move(testGameObject2));

  auto mainLight = LveGameObject::makePointLight(10.0f);
  mainLight.color = {
      1.0f,
      1.0f,
      1.0f
  };

  mainLight.transform.translation = {0.0f, -5.0f, 0.0f};
  mainLight.transform.scale = {1.0f, 1.0f, 1.0f};
  gameObjects.emplace(mainLight.getId(), std::move(mainLight));
}

}  // namespace lve
