#include "data/chunk.hpp"
#include "data/world.hpp"
#include "voxel_app.hpp"
#include "graphics/face_culling_chunk_mesher.hpp"
#include "fps_movement_controller.hpp"
#include "./engine/buffer.hpp"
#include "./engine/descriptors.hpp"

#include "./engine/descriptor_set_layout_builder.hpp"
#include "lve_camera.hpp"
#include "./engine/camera.hpp"
#include "./engine/fps_movement_controller.hpp"
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

  loadGameObjects();
}

VoxelApp::~VoxelApp() {}

void VoxelApp::run() {
  std::vector<std::unique_ptr<engine::Buffer>> uboBuffers(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<engine::Buffer>(
        lveDevice.device(),
        lveDevice.getPhysicalDevice(),
        sizeof(GlobalUbo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible);
    uboBuffers[i]->map();
  }

  std::vector<vk::DescriptorSetLayoutBinding> bindings;
  auto globalSetLayout =
  engine::DescriptorSetLayoutBuilder(lveDevice.device())
      .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
      .addBinding(
          vk::DescriptorType::eCombinedImageSampler,
          1,
          vk::ShaderStageFlagBits::eFragment)
      .build(bindings);

  std::unique_ptr<Image> image = std::make_unique<Image>(lveDevice, "../textures/hinoki_planks_diff_4k.jpg");
  //std::unique_ptr<Image> image = std::make_unique<Image>(lveDevice, "../textures/metal_plate_diff_4k.jpg");
  
  vk::DescriptorImageInfo imageInfo = vk::DescriptorImageInfo()
    .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
    .setImageView(image->getImageView())
    .setSampler(image->getSampler());

  std::vector<VkDescriptorSet> globalDescriptorSets(LveSwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    globalDescriptorSets[i] = engine::DescriptorWriter(
        lveDevice.device(), 
        globalPool->getPoolCpp(),
        globalSetLayout,
        bindings
      )
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imageInfo)
        .build();
  }

  SimpleRenderSystem simpleRenderSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(), globalSetLayout};
  PointLightSystem pointLightSystem{
      lveDevice,
      lveRenderer.getSwapChainRenderPass(), globalSetLayout};
  LveCamera camera{};

  auto viewerObject = LveGameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  FpsMovementController cameraController{_window};

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (!_window.shouldClose()) {
    _window.pollEvents();

    if (_window.isKeyPressed(engine::KeyboardKey::ESCAPE)) {
      _window.close();
    }

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.updateView(_window, frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = lveRenderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);

    if (auto commandBuffer = lveRenderer.beginFrame()) {
      int frameIndex = lveRenderer.getFrameIndex();

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
  auto testGameObject = LveGameObject::createGameObject();
  glm::vec3 position = {1.0f, 1.0f, 1.0f};
  auto testMesh = createCubeMesh(position, {0.0f, 0.5f, 0.5f}, true, true, true, true, true, true);
  auto commandBuffer = lveDevice.beginSingleTimeCommands();
  std::shared_ptr<LveModel> testModel = std::make_shared<LveModel>(lveDevice.device(), lveDevice.getPhysicalDevice(), commandBuffer, testMesh);
  testGameObject.model = testModel;
  testGameObject.transform.translation = position;  // chunk.Position * (float)chunk.Size;
  testGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
  gameObjects.emplace(testGameObject.getId(), std::move(testGameObject));

  auto testGameObject2 = LveGameObject::createGameObject();
  glm::vec3 position2 = {2.0f, 1.0f, 1.0f};
  auto testMesh2 = createCubeMesh(position2, {1.0f, 0.65f, 0.0f}, true, true, true, true, true, true);
  std::shared_ptr<LveModel> testModel2 = std::make_shared<LveModel>(lveDevice.device(), lveDevice.getPhysicalDevice(), commandBuffer, testMesh2);
  testGameObject2.model = testModel2;
  testGameObject2.transform.translation = position2;  // chunk.Position * (float)chunk.Size;
  testGameObject2.transform.scale = {2.0f, 2.0f, 2.0f};
  gameObjects.emplace(testGameObject2.getId(), std::move(testGameObject2));
  lveDevice.endSingleTimeCommands(commandBuffer);

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
