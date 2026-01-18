#include "raii_app.hpp"

#include "data/chunk.hpp"
#include "data/world.hpp"
#include "fps_movement_controller_typed.hpp"
#include "graphics/face_culling_chunk_mesher.hpp"
#include "graphics/graphics_util.hpp"
#include "lve_buffer.hpp"
#include "lve_camera.hpp"
#include "systems/point_light_system_typed.hpp"
#include "systems/simple_render_system_typed.hpp"
#include "model.hpp"

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <image_raii.hpp>
#include <keyboard_movement_controller.hpp>
#include <stdexcept>
#include <swap_chain.hpp>

namespace lve {

RaiiApp::RaiiApp() {
  window = std::make_shared<Window>(WIDTH, HEIGHT, "Vulkan RAII Voxel Engine");
  device = std::make_shared<Device>(*window.get());
  renderer = std::make_shared<Renderer>(*window.get(), device);
  createDescriptorPool();
  
  loadGameObjects();
}

void RaiiApp::createDescriptorPool() {
  vk::DescriptorPoolSize a(vk::DescriptorType::eUniformBuffer, SwapChain::MAX_FRAMES_IN_FLIGHT);
  vk::DescriptorPoolSize b(
      vk::DescriptorType::eCombinedImageSampler,
      SwapChain::MAX_FRAMES_IN_FLIGHT);
  std::vector<vk::DescriptorPoolSize> sizes = {a /*, b*/};
  // TODO: Wrap in a class/method for ease use
  vk::DescriptorPoolCreateInfo descriptorPoolInfo =
      vk::DescriptorPoolCreateInfo().setPoolSizeCount(1).setPoolSizes(sizes).setMaxSets(
          SwapChain::MAX_FRAMES_IN_FLIGHT)
      .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
  //assert(descriptorPoolCreateInfo.flags & vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);
  globalPool =
      std::make_shared<vk::raii::DescriptorPool>(device->createDescriptorPool(descriptorPoolInfo));
}

RaiiApp::~RaiiApp() {}

void RaiiApp::createDescriptorSetLayout() {

  vk::DescriptorSetLayoutBinding uniformBuffer =
      vk::DescriptorSetLayoutBinding()
          .setBinding(0)
          .setDescriptorType(vk::DescriptorType::eUniformBuffer)
          .setDescriptorCount(1)
          .setStageFlags(vk::ShaderStageFlagBits::eAllGraphics);

  vk::DescriptorSetLayoutBinding imageSamplerBuffer =
      vk::DescriptorSetLayoutBinding()
      .setBinding(1)
      .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
      .setDescriptorCount(1)
      .setStageFlags(vk::ShaderStageFlagBits::eFragment);
  setLayoutBindings.push_back(uniformBuffer);
  setLayoutBindings.push_back(imageSamplerBuffer);
  vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutInfo =
      vk::DescriptorSetLayoutCreateInfo()
          .setBindingCount(static_cast<uint32_t>(setLayoutBindings.size()))
          .setBindings(setLayoutBindings);
  descriptorSetLayout =
      std::make_shared<vk::raii::DescriptorSetLayout>(device->device(), descriptorSetLayoutInfo);
}

void RaiiApp::run() {
  std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<Buffer>(
        device,
        sizeof(GlobalUbo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible);
    uboBuffers[i]->map();
  }
  createDescriptorSetLayout();

  // TODO: Image needs to be created here instead of attached to the LveGameObject

  std::unique_ptr<ImageRaii> image =
      std::make_unique<ImageRaii>(device, "textures/jungle-brick-with-moss.png");
  vk::DescriptorImageInfo imageInfo{};
  imageInfo.imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
  imageInfo.imageView = image->getImageView();
  imageInfo.sampler = image->getSampler();

  // std::vector<std::shared_ptr<vk::raii::DescriptorSet>>
  // globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
  
  //std::array<const vk::raii::DescriptorSetLayout&, 2> layouts = {**descriptorSetLayout, **descriptorSetLayout};
  //vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(*globalPool, layouts);
  std::vector<vk::DescriptorSetLayout> layouts = {
      **descriptorSetLayout,
      **descriptorSetLayout};
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo(*globalPool, layouts);

  vk::raii::DescriptorSets globalDescriptorSets(device->device(), descriptorSetAllocateInfo);


  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    // TODO: Add Image buffer back into this
   
    const std::array<vk::WriteDescriptorSet, 2> writeDescriptorSet = {
        vk::WriteDescriptorSet()
        .setDstSet(*globalDescriptorSets[i])
        .setDstBinding(0)
        .setDescriptorCount(1)
        .setDescriptorType(
            vk::DescriptorType::eUniformBuffer)
        .setBufferInfo(bufferInfo),
        vk::WriteDescriptorSet()
        .setDstSet(*globalDescriptorSets[i])
        .setDstBinding(1)
        .setDescriptorCount(1)
        .setDescriptorType(
            vk::DescriptorType::eCombinedImageSampler)
        .setImageInfo(imageInfo)};
    device->device().updateDescriptorSets(writeDescriptorSet, {});
  }

  SimpleRenderSystemTyped simpleRenderSystem{
      device,
      renderer->getSwapChainRenderPass(),
      *descriptorSetLayout};
    
  PointLightSystemTyped pointLightSystem{
      device,
      renderer->getSwapChainRenderPass(),
      *descriptorSetLayout};
  
  LveCamera camera{};

  auto viewerObject = GameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  FpsMovementControllerTyped cameraController{window};
  
  auto currentTime = std::chrono::high_resolution_clock::now();

  while (!window->shouldClose()) {
    window->pollEvents();
   
    if (window->isKeyPressed(KeyboardKey::ESCAPE)) {
      window->close();
    }

    
    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.updateView(window, frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = renderer->getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 100.f);
    
    bool hasFrame = true;
    const vk::raii::CommandBuffer& commandBuffer = renderer->beginFrame(hasFrame);
    if (hasFrame) {
      int frameIndex = renderer->getFrameIndex();

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
          *globalDescriptorSets[frameIndex],
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
      renderer->beginSwapChainRenderPass(commandBuffer);

      // order here matters
      simpleRenderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);

      renderer->endSwapChainRenderPass(commandBuffer);
      renderer->endFrame();
    }
    
  }
  device->waitIdle();
}

void RaiiApp::loadGameObjects() {
  auto testGameObject = GameObject::createGameObject();
  glm::vec3 position = {1.0f, 1.0f, 1.0f};
  auto testMesh = createCubeMesh(position, {0.0f, 0.5f, 0.5f}, true, true, true, true, true, true);
  std::shared_ptr<Model> testModel = std::make_shared<Model>(device, testMesh);
  testGameObject.model = testModel;
  testGameObject.transform.translation = position;  // chunk.Position * (float)chunk.Size;
  testGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
  gameObjects.emplace(testGameObject.getId(), std::move(testGameObject));
  
  auto testGameObject2 = GameObject::createGameObject();
  glm::vec3 position2 = {2.0f, 1.0f, 1.0f};
  auto testMesh2 =
      createCubeMesh(position2, {1.0f, 0.65f, 0.0f}, true, true, true, true, true, true);
  std::shared_ptr<Model> testModel2 = std::make_shared<Model>(device, testMesh2);
  testGameObject2.model = testModel2;
  testGameObject2.transform.translation = position2;  // chunk.Position * (float)chunk.Size;
  testGameObject2.transform.scale = {0.2f, 0.2f, 0.2f};
  gameObjects.emplace(testGameObject2.getId(), std::move(testGameObject2));

  auto mainLight = GameObject::makePointLight(10.0f);
  mainLight.color = {1.0f, 1.0f, 1.0f};

  mainLight.transform.translation = {0.0f, -5.0f, 0.0f};
  mainLight.transform.scale = {1.0f, 1.0f, 1.0f};
  gameObjects.emplace(mainLight.getId(), std::move(mainLight));
}
}  // namespace lve
