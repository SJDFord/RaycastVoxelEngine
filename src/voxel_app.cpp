#include "voxel_app.hpp"
#include "./engine/utils.hpp"
#include "./engine/graphics/vulkan/buffer.hpp"
#include "./engine/graphics/vulkan/descriptors.hpp"
#include "./engine/graphics/vulkan/model.hpp"

#include "./engine/graphics/vulkan/builders/descriptor_set_layout_bindings_builder.hpp"
#include "./engine/logic/camera.hpp"
#include "./engine/logic/fps_movement_controller.hpp"
#include "systems/point_light_system.hpp"
#include "systems/simple_render_system.hpp"
#include "systems/skybox_render_system.hpp"
#include "./engine/graphics/vulkan/builders/descriptor_pool_create_info_builder.hpp"
#include "./engine/graphics/vulkan/builders/file_image_builder.hpp"
#include "./engine/graphics/vulkan/builders/file_model_builder.hpp"

#include "./engine/graphics/vulkan/image.hpp"
#include "./engine/graphics/layout/simple.hpp"
#include <utility>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <ktx.h>
#include <ktxvulkan.h>

// std
#include <array>
#include <cassert>
#include <chrono>
#include <stdexcept>

namespace lve {

VoxelApp::VoxelApp(): 
  _globalPool{_device.createDescriptorPool(
    engine::DescriptorPoolCreateInfoBuilder()
      .setMaxSets(MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
      .build()
  )},
  _skyboxPool{_device.createDescriptorPool(
    engine::DescriptorPoolCreateInfoBuilder()
      .setMaxSets(MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT)
      .addPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
      .build()
  )}
  
  {
  /*
  vkCmdBeginRenderingKHR = reinterpret_cast<PFN_vkCmdBeginRenderingKHR>(vkGetInstanceProcAddr(_instance, "vkCmdBeginRenderingKHR"));
  vkCmdEndRenderingKHR   = reinterpret_cast<PFN_vkCmdEndRenderingKHR>(vkGetInstanceProcAddr(_instance, "vkCmdEndRenderingKHR"));
  if (!vkCmdBeginRenderingKHR || !vkCmdEndRenderingKHR)
  {
    throw std::runtime_error("Unable to dynamically load vkCmdBeginRenderingKHR and vkCmdEndRenderingKHR");
  }
  */
  loadGameObjects();
}

VoxelApp::~VoxelApp() {
  _device.destroyDescriptorPool(_globalPool);
}

void VoxelApp::run() {
	vk::SamplerCreateInfo samplerCreateInfo = vk::SamplerCreateInfo()
		.setMagFilter(vk::Filter::eNearest)
		.setMinFilter(vk::Filter::eNearest)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
    .setAnisotropyEnable(vk::False)
		.setMaxAnisotropy(0.0f)
		.setCompareOp(vk::CompareOp::eNever)
		.setMinLod(0.0f)
		.setMaxLod(0.0f)
		.setBorderColor(vk::BorderColor::eFloatOpaqueWhite);

	auto globalSampler = _device.createSampler(samplerCreateInfo);


  std::vector<std::unique_ptr<engine::Buffer>> skyboxUboBuffers(MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < skyboxUboBuffers.size(); i++) {
    skyboxUboBuffers[i] = std::make_unique<engine::Buffer>(
        _device,
        _physicalDevice,
        sizeof(engine::GlobalUbo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible);
    skyboxUboBuffers[i]->map();
  }


  std::vector<std::unique_ptr<engine::Buffer>> uboBuffers(MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<engine::Buffer>(
        _device,
        _physicalDevice,
        sizeof(engine::GlobalUbo),
        1,
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible);
    uboBuffers[i]->map();
  }



  // https://kylehalladay.com/blog/tutorial/vulkan/2018/01/28/Textue-Arrays-Vulkan.html
  std::vector<vk::DescriptorSetLayoutBinding> bindings = engine::DescriptorSetLayoutBindingsBuilder()
    .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
    .addBinding(vk::DescriptorType::eSampler, 1, vk::ShaderStageFlagBits::eFragment)
    .addBinding(vk::DescriptorType::eSampledImage, MAX_TEXTURE_COUNT, vk::ShaderStageFlagBits::eFragment)
    //.addBinding(vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
    .build();
    
  std::vector<vk::DescriptorSetLayoutBinding> skyboxBindings = engine::DescriptorSetLayoutBindingsBuilder()
    .addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
    .addBinding(vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
    .build();

  vk::DescriptorSetLayout globalSetLayout = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings));


  vk::DescriptorSetLayout skyboxSetLayout = _device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, skyboxBindings));

  std::unique_ptr<engine::Image> imageGrass = engine::FileImageBuilder(
    _device, 
    _physicalDevice
  ).build(
    _oneTimeCommandSubmitter, 
    //"../textures/hinoki_planks_diff_4k.jpg"
    "../textures/ground_dry_rgba.ktx"
  );

  std::unique_ptr<engine::Image> imageWood = engine::FileImageBuilder(
    _device, 
    _physicalDevice
  ).build(
    _oneTimeCommandSubmitter, 
    "../textures/hinoki_planks_diff_4k.jpg"
  );

  std::unique_ptr<engine::Image> imageMetal = engine::FileImageBuilder(
    _device, 
    _physicalDevice
  ).build(
    _oneTimeCommandSubmitter, 
    "../textures/metalplate01_rgba.ktx"
  );


  std::unique_ptr<engine::Image> skyboxCubeMap = engine::FileImageBuilder(
    _device, 
    _physicalDevice
  ).build(
    _oneTimeCommandSubmitter, 
    //"../textures/hinoki_planks_diff_4k.jpg"
    "../textures/skybox/cubemap_yokohama_rgba.ktx",
    true
  );


  std::vector<std::shared_ptr<engine::Image>> skyboxImages;
  auto skyboxGameObject = engine::GameObject::createGameObject();
  glm::vec3 position = {1.0f, 1.0f, 1.0f};
  skyboxGameObject.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/cube.gltf", skyboxImages);
  skyboxGameObject.transform.translation = position;
  skyboxGameObject.transform.scale = {10.0f, 10.0f, 10.0f};
  engine::GameObject::Map skyboxObjects;
  skyboxObjects.emplace(skyboxGameObject.getId(), std::move(skyboxGameObject));

  vk::DescriptorImageInfo imageInfoDescriptors[MAX_TEXTURE_COUNT];
  
  for (uint32_t i = 0; i < globalImages.size(); ++i) {
		imageInfoDescriptors[i].sampler = nullptr;
		imageInfoDescriptors[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    imageInfoDescriptors[i].imageView = globalImages[i]->GetImageView();
  }

  
  /*
  for (uint32_t i = globalImages.size(); i < MAX_TEXTURE_COUNT; i++) {
		demoData.descriptorImageInfos[i].sampler = nullptr;
		demoData.descriptorImageInfos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		demoData.descriptorImageInfos[i].imageView = imageGrass->GetImageView();
  }
  */

  /*
  for (uint32_t i = 0; i < 8; ++i)
	{
		demoData.descriptorImageInfos[i].sampler = nullptr;
		demoData.descriptorImageInfos[i].imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    if (i == 0) {

		  demoData.descriptorImageInfos[i].imageView = imageGrass->GetImageView();
    } else if (i == 1) {

		  demoData.descriptorImageInfos[i].imageView = imageWood->GetImageView();
    } else if (i == 2) {

		  demoData.descriptorImageInfos[i].imageView = imageMetal->GetImageView();
    }
	}
  */

  auto skyboxCubeMapInfo = skyboxCubeMap->descriptorInfo();
  // TODO: Create multiple descriptor sets for each frame, following: https://vkguide.dev/docs/chapter-4/descriptors/#mental-model
  // TODO: Tie this in with Model loading
  std::vector<vk::DescriptorSet> skyboxDescriptorSets(MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < skyboxDescriptorSets.size(); i++) {
    auto bufferInfo = skyboxUboBuffers[i]->descriptorInfo();
    skyboxDescriptorSets[i] = engine::DescriptorWriter(
      _device, 
      _skyboxPool,
      skyboxSetLayout,
      skyboxBindings
    )
      .writeBuffer(0, &bufferInfo)
      .writeImage(1, &skyboxCubeMapInfo)
      .build();
  }
  
  vk::DescriptorImageInfo samplerInfo = {};
	samplerInfo.sampler = globalSampler;
  std::vector<vk::DescriptorSet> globalDescriptorSets(MAX_FRAMES_IN_FLIGHT);
  for (int i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    globalDescriptorSets[i] = engine::DescriptorWriter(
      _device, 
      _globalPool,
      globalSetLayout,
      bindings
    )
      .writeBuffer(0, &bufferInfo)
      .writeImage(1, &samplerInfo)
      .writeImage(2, imageInfoDescriptors, MAX_TEXTURE_COUNT)
      .build();
  }

  SkyboxRenderSystem skyboxRenderSystem{
      _device,
      _renderer.getSwapChainRenderPass(),
      _renderer.getSwapChainColorFormat(),
      _renderer.getSwapChainDepthFormat(),
      skyboxSetLayout
  };
  SimpleRenderSystem simpleRenderSystem{
      _device,
      _renderer.getSwapChainRenderPass(),
      _renderer.getSwapChainColorFormat(),
      _renderer.getSwapChainDepthFormat(),
       globalSetLayout
  };
  PointLightSystem pointLightSystem{
      _device,
      _renderer.getSwapChainRenderPass(),
      _renderer.getSwapChainColorFormat(),
      _renderer.getSwapChainDepthFormat(),
      globalSetLayout
  };
  engine::Camera camera{};

  auto viewerObject = engine::GameObject::createGameObject();
  viewerObject.transform.translation.z = -2.5f;
  engine::FpsMovementController cameraController{_window};


  bool isRunning = true;

  _window.addKeyListener([&] (engine::KeyboardKey key, bool isPressed, vk::Flags<engine::ModifierFlagBits> modifierFlags) {
    if (key == engine::KeyboardKey::ESCAPE && isPressed) {
      isRunning = false;
    }
  });
  _window.addCloseListener([&] () {
    isRunning = false;
  });

  auto currentTime = std::chrono::high_resolution_clock::now();
  while (isRunning) {
    _window.pollEvents();

    auto newTime = std::chrono::high_resolution_clock::now();
    float frameTime =
        std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
    currentTime = newTime;

    cameraController.updateView(_window, frameTime, viewerObject);
    camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

    float aspect = _renderer.getAspectRatio();
    camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 500.f);

    _renderer.renderFrame(
      [&] (const vk::CommandBuffer& commandBuffer, int frameIndex) { 
        engine::FrameInfo frameInfo{
          frameIndex,
          frameTime,
          commandBuffer,
          camera,
          globalDescriptorSets[frameIndex],
          gameObjects
        };

        auto mainLight = engine::GameObject::makePointLight(10.0f);
        mainLight.color = {1.0f, 1.0f, 1.0f};

        glm::vec3 lightOffset = {0.0f, 2.0f, 0.0f};
        mainLight.transform.translation = viewerObject.transform.translation + lightOffset;
        mainLight.transform.scale = {0.1f, 0.1f, 0.1f};

        // update
        engine::GlobalUbo ubo{};
        ubo.projection = camera.getProjection();
        ubo.view = camera.getView();
        ubo.inverseView = camera.getInverseView();
        pointLightSystem.update(frameInfo, ubo);
        uboBuffers[frameIndex]->writeToBuffer(&ubo);
        uboBuffers[frameIndex]->flush();

        skyboxUboBuffers[frameIndex]->writeToBuffer(&ubo);
        skyboxUboBuffers[frameIndex]->flush();
      }, 
      {
        [&] (const vk::CommandBuffer& commandBuffer, int frameIndex) {

          engine::FrameInfo skyboxFrameInfo{
            frameIndex,
            frameTime,
            commandBuffer,
            camera,
            skyboxDescriptorSets[frameIndex],
            skyboxObjects
          };
          skyboxRenderSystem.renderGameObjects(skyboxFrameInfo);
          engine::FrameInfo frameInfo{
            frameIndex,
            frameTime,
            commandBuffer,
            camera,
            globalDescriptorSets[frameIndex],
            gameObjects
          };
          simpleRenderSystem.renderGameObjects(frameInfo);
          pointLightSystem.render(frameInfo);
        }  
      },
      [&] (const vk::CommandBuffer& commandBuffer, int frameIndex) { 

      }
    );
  }
  
  std::println("Cleanup");
  _device.destroySampler(globalSampler);
  _device.waitIdle();
}

void VoxelApp::loadGameObjects() {

  auto testGameObject = engine::GameObject::createGameObject();
  glm::vec3 position = {1.0f, 1.0f, 1.0f};
  testGameObject.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/cube.gltf", globalImages);
  testGameObject.transform.translation = position;
  testGameObject.transform.scale = {0.2f, 0.2f, 0.2f};
  gameObjects.emplace(testGameObject.getId(), std::move(testGameObject));

  auto testGameObject2 = engine::GameObject::createGameObject();
  glm::vec3 position2 = {2.0f, 1.0f, 1.0f};
  engine::Mesh test2Mesh = engine::createCubeMesh(position2, {1.0f, 0.65f, 0.0f}, true, true, true, true, true, true);
  std::vector<std::pair<engine::Mesh, int>> pairs = {{test2Mesh, 19}};
  std::shared_ptr<engine::Model> testModel2 = std::make_shared<engine::Model>(_device, _physicalDevice, _oneTimeCommandSubmitter, pairs);
  testGameObject2.model = testModel2;
  testGameObject2.transform.translation = position2;
  testGameObject2.transform.scale = {2.0f, 2.0f, 2.0f};
  gameObjects.emplace(testGameObject2.getId(), std::move(testGameObject2));
  
  auto testGameObject3 = engine::GameObject::createGameObject();
  glm::vec3 position3 = {-20.0f, 1.0f, -10.0f};
  testGameObject3.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/chinesedragon.gltf", globalImages);
  testGameObject3.transform.translation = position3;
  testGameObject3.transform.scale = {1.0f, 1.0f, 1.0f};
  testGameObject3.transform.rotation = {-glm::pi<float>() / 2.0f, 0.0f, 0.0f};
  gameObjects.emplace(testGameObject3.getId(), std::move(testGameObject3));
  

  auto testGameObject4 = engine::GameObject::createGameObject();
  glm::vec3 position4 = {20.0f, 1.0f, -10.0f};
  testGameObject4.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/oaktree.gltf", globalImages);
  testGameObject4.transform.translation = position4;
  testGameObject4.transform.scale = {10.0f, 10.0f, 10.0f};
  testGameObject4.transform.rotation = {-glm::pi<float>() / 2.0f, 0.0f, 0.0f};
  gameObjects.emplace(testGameObject4.getId(), std::move(testGameObject4));

  auto testGameObject5 = engine::GameObject::createGameObject();
  glm::vec3 position5 = {20.0f, 1.0f, -30.0f};
  testGameObject5.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/FlightHelmet/glTF/FlightHelmet.gltf", globalImages);
  testGameObject5.model->setName("helmet");
  testGameObject5.transform.translation = position5;
  testGameObject5.transform.scale = {4.0f, 4.0f, 4.0f};
  testGameObject5.transform.rotation = {glm::pi<float>(), 0.0f, 0.0f};
  gameObjects.emplace(testGameObject5.getId(), std::move(testGameObject5));


  /*
  auto testGameObject6 = engine::GameObject::createGameObject();
  glm::vec3 position6 = {0.0f, 1.0f, -30.0f};
  testGameObject6.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/deer.gltf", globalImages);
  testGameObject6.transform.translation = position6;
  testGameObject6.transform.scale = {4.0f, 4.0f, 4.0f};
  testGameObject6.transform.rotation = {glm::pi<float>(), 0.0f, 0.0f};
  gameObjects.emplace(testGameObject6.getId(), std::move(testGameObject6));
  */

  
  auto testGameObject7 = engine::GameObject::createGameObject();
  glm::vec3 position7 = {-20.0f, 2.0f, -30.0f};
  testGameObject7.model = engine::FileModelBuilder(_device, _physicalDevice).build(_oneTimeCommandSubmitter, "../models/Sponza/glTF/Sponza.gltf", globalImages);
  testGameObject7.transform.translation = position7;
  testGameObject7.transform.scale = {0.25f, 0.25f, 0.25f};
  testGameObject7.transform.rotation = {glm::pi<float>(), 0.0f, 0.0f};
  gameObjects.emplace(testGameObject7.getId(), std::move(testGameObject7));
  

  auto mainLight = engine::GameObject::makePointLight(10.0f);
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
