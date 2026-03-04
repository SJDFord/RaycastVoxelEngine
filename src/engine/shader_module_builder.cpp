#include "shader_module_builder.hpp"

namespace engine {

ShaderModuleBuilder::ShaderModuleBuilder(vk::Device const& device, vk::ShaderStageFlagBits shaderStage, std::string glslCode) {
  _device = device;
  _shaderStage = shaderStage;
  _glslCode = glslCode;
}

ShaderModuleBuilder::~ShaderModuleBuilder() {}

/*
ShaderModuleBuilder& ShaderModuleBuilder::addBinding(
    vk::DescriptorType descriptorType, uint32_t descriptorCount, vk::ShaderStageFlags stageFlags) {
  _bindingData.push_back({descriptorType, descriptorCount, stageFlags});
  return *this;
}
ShaderModuleBuilder& ShaderModuleBuilder::setFlags(
    vk::DescriptorSetLayoutCreateFlags flags) {
  _flags = flags;
  return *this;
}
*/

vk::ShaderModule ShaderModuleBuilder::build() {
  glslang::InitializeProcess();
  std::vector<unsigned int> shaderSPV;
  if (!GLSLtoSPV(_shaderStage, _glslCode, shaderSPV)) {
    throw std::runtime_error("Could not convert glsl shader to spir-v -> terminating");
  }

  vk::ShaderModule shaderModule = _device.createShaderModule(
      vk::ShaderModuleCreateInfo(vk::ShaderModuleCreateFlags(), shaderSPV));
  glslang::FinalizeProcess();

  return shaderModule;
}

EShLanguage ShaderModuleBuilder::translateShaderStage(vk::ShaderStageFlagBits stage) {
  switch (stage) {
    case vk::ShaderStageFlagBits::eVertex:
      return EShLangVertex;
    case vk::ShaderStageFlagBits::eTessellationControl:
      return EShLangTessControl;
    case vk::ShaderStageFlagBits::eTessellationEvaluation:
      return EShLangTessEvaluation;
    case vk::ShaderStageFlagBits::eGeometry:
      return EShLangGeometry;
    case vk::ShaderStageFlagBits::eFragment:
      return EShLangFragment;
    case vk::ShaderStageFlagBits::eCompute:
      return EShLangCompute;
    case vk::ShaderStageFlagBits::eRaygenNV:
      return EShLangRayGenNV;
    case vk::ShaderStageFlagBits::eAnyHitNV:
      return EShLangAnyHitNV;
    case vk::ShaderStageFlagBits::eClosestHitNV:
      return EShLangClosestHitNV;
    case vk::ShaderStageFlagBits::eMissNV:
      return EShLangMissNV;
    case vk::ShaderStageFlagBits::eIntersectionNV:
      return EShLangIntersectNV;
    case vk::ShaderStageFlagBits::eCallableNV:
      return EShLangCallableNV;
    case vk::ShaderStageFlagBits::eTaskNV:
      return EShLangTaskNV;
    case vk::ShaderStageFlagBits::eMeshNV:
      return EShLangMeshNV;
    default:
      assert(false && "Unknown shader stage");
      return EShLangVertex;
  }
}

TBuiltInResource ShaderModuleBuilder::InitResources() {
  TBuiltInResource Resources;

  Resources.maxLights = 32;
  Resources.maxClipPlanes = 6;
  Resources.maxTextureUnits = 32;
  Resources.maxTextureCoords = 32;
  Resources.maxVertexAttribs = 64;
  Resources.maxVertexUniformComponents = 4096;
  Resources.maxVaryingFloats = 64;
  Resources.maxVertexTextureImageUnits = 32;
  Resources.maxCombinedTextureImageUnits = 80;
  Resources.maxTextureImageUnits = 32;
  Resources.maxFragmentUniformComponents = 4096;
  Resources.maxDrawBuffers = 32;
  Resources.maxVertexUniformVectors = 128;
  Resources.maxVaryingVectors = 8;
  Resources.maxFragmentUniformVectors = 16;
  Resources.maxVertexOutputVectors = 16;
  Resources.maxFragmentInputVectors = 15;
  Resources.minProgramTexelOffset = -8;
  Resources.maxProgramTexelOffset = 7;
  Resources.maxClipDistances = 8;
  Resources.maxComputeWorkGroupCountX = 65535;
  Resources.maxComputeWorkGroupCountY = 65535;
  Resources.maxComputeWorkGroupCountZ = 65535;
  Resources.maxComputeWorkGroupSizeX = 1024;
  Resources.maxComputeWorkGroupSizeY = 1024;
  Resources.maxComputeWorkGroupSizeZ = 64;
  Resources.maxComputeUniformComponents = 1024;
  Resources.maxComputeTextureImageUnits = 16;
  Resources.maxComputeImageUniforms = 8;
  Resources.maxComputeAtomicCounters = 8;
  Resources.maxComputeAtomicCounterBuffers = 1;
  Resources.maxVaryingComponents = 60;
  Resources.maxVertexOutputComponents = 64;
  Resources.maxGeometryInputComponents = 64;
  Resources.maxGeometryOutputComponents = 128;
  Resources.maxFragmentInputComponents = 128;
  Resources.maxImageUnits = 8;
  Resources.maxCombinedImageUnitsAndFragmentOutputs = 8;
  Resources.maxCombinedShaderOutputResources = 8;
  Resources.maxImageSamples = 0;
  Resources.maxVertexImageUniforms = 0;
  Resources.maxTessControlImageUniforms = 0;
  Resources.maxTessEvaluationImageUniforms = 0;
  Resources.maxGeometryImageUniforms = 0;
  Resources.maxFragmentImageUniforms = 8;
  Resources.maxCombinedImageUniforms = 8;
  Resources.maxGeometryTextureImageUnits = 16;
  Resources.maxGeometryOutputVertices = 256;
  Resources.maxGeometryTotalOutputComponents = 1024;
  Resources.maxGeometryUniformComponents = 1024;
  Resources.maxGeometryVaryingComponents = 64;
  Resources.maxTessControlInputComponents = 128;
  Resources.maxTessControlOutputComponents = 128;
  Resources.maxTessControlTextureImageUnits = 16;
  Resources.maxTessControlUniformComponents = 1024;
  Resources.maxTessControlTotalOutputComponents = 4096;
  Resources.maxTessEvaluationInputComponents = 128;
  Resources.maxTessEvaluationOutputComponents = 128;
  Resources.maxTessEvaluationTextureImageUnits = 16;
  Resources.maxTessEvaluationUniformComponents = 1024;
  Resources.maxTessPatchComponents = 120;
  Resources.maxPatchVertices = 32;
  Resources.maxTessGenLevel = 64;
  Resources.maxViewports = 16;
  Resources.maxVertexAtomicCounters = 0;
  Resources.maxTessControlAtomicCounters = 0;
  Resources.maxTessEvaluationAtomicCounters = 0;
  Resources.maxGeometryAtomicCounters = 0;
  Resources.maxFragmentAtomicCounters = 8;
  Resources.maxCombinedAtomicCounters = 8;
  Resources.maxAtomicCounterBindings = 1;
  Resources.maxVertexAtomicCounterBuffers = 0;
  Resources.maxTessControlAtomicCounterBuffers = 0;
  Resources.maxTessEvaluationAtomicCounterBuffers = 0;
  Resources.maxGeometryAtomicCounterBuffers = 0;
  Resources.maxFragmentAtomicCounterBuffers = 1;
  Resources.maxCombinedAtomicCounterBuffers = 1;
  Resources.maxAtomicCounterBufferSize = 16384;
  Resources.maxTransformFeedbackBuffers = 4;
  Resources.maxTransformFeedbackInterleavedComponents = 64;
  Resources.maxCullDistances = 8;
  Resources.maxCombinedClipAndCullDistances = 8;
  Resources.maxSamples = 4;
  Resources.maxMeshOutputVerticesNV = 256;
  Resources.maxMeshOutputPrimitivesNV = 512;
  Resources.maxMeshWorkGroupSizeX_NV = 32;
  Resources.maxMeshWorkGroupSizeY_NV = 1;
  Resources.maxMeshWorkGroupSizeZ_NV = 1;
  Resources.maxTaskWorkGroupSizeX_NV = 32;
  Resources.maxTaskWorkGroupSizeY_NV = 1;
  Resources.maxTaskWorkGroupSizeZ_NV = 1;
  Resources.maxMeshViewCountNV = 4;

  Resources.limits.nonInductiveForLoops = 1;
  Resources.limits.whileLoops = 1;
  Resources.limits.doWhileLoops = 1;
  Resources.limits.generalUniformIndexing = 1;
  Resources.limits.generalAttributeMatrixVectorIndexing = 1;
  Resources.limits.generalVaryingIndexing = 1;
  Resources.limits.generalSamplerIndexing = 1;
  Resources.limits.generalVariableIndexing = 1;
  Resources.limits.generalConstantMatrixVectorIndexing = 1;

  return Resources;
}

bool ShaderModuleBuilder::GLSLtoSPV(
    const vk::ShaderStageFlagBits shaderType,
    std::string const& glslShader,
    std::vector<unsigned int>& spvShader) {
  EShLanguage stage = translateShaderStage(shaderType);

  const char* shaderStrings[1];
  shaderStrings[0] = glslShader.data();

  glslang::TShader shader(stage);
  shader.setStrings(shaderStrings, 1);

  // Enable SPIR-V and Vulkan rules when parsing GLSL
  EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
  // shader.parse
  TBuiltInResource resources(InitResources());
  if (!shader.parse(&resources, 100, false, messages)) {
    puts(shader.getInfoLog());
    puts(shader.getInfoDebugLog());
    return false;  // something didn't work
  }

  glslang::TProgram program;
  program.addShader(&shader);

  //
  // Program-level processing...
  //

  if (!program.link(messages)) {
    puts(shader.getInfoLog());
    puts(shader.getInfoDebugLog());
    fflush(stdout);
    return false;
  }

  glslang::GlslangToSpv(*program.getIntermediate(stage), spvShader);
  return true;
}

}  // namespace engine

