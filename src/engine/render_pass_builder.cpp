#include "render_pass_builder.hpp"

namespace engine {

RenderPassBuilder::RenderPassBuilder(vk::Device const& device) {
  _device = device;
}

RenderPassBuilder::~RenderPassBuilder() {}



RenderPassBuilder& RenderPassBuilder::setColorFormat(vk::Format colorFormat) {
  _colorFormat = colorFormat;
  return *this;
}
RenderPassBuilder& RenderPassBuilder::setDepthFormat(vk::Format depthFormat) {
  _depthFormat = depthFormat;
  return *this;
}
vk::RenderPass  RenderPassBuilder::build() {
  vk::AttachmentDescription depthAttachment = vk::AttachmentDescription()
    .setFormat(_depthFormat)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setLoadOp(vk::AttachmentLoadOp::eClear)
    .setStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

  vk::AttachmentReference depthAttachmentRef = vk::AttachmentReference()
    .setAttachment(1)
    .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

  vk::AttachmentDescription colorAttachment = vk::AttachmentDescription()
    .setFormat(_colorFormat)
    .setSamples(vk::SampleCountFlagBits::e1)
    .setLoadOp(vk::AttachmentLoadOp::eClear)
    .setStoreOp(vk::AttachmentStoreOp::eStore)
    .setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
    .setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
    .setInitialLayout(vk::ImageLayout::eUndefined)
    .setFinalLayout(vk::ImageLayout::ePresentSrcKHR);

  vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference()
    .setAttachment(0)
    .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

  vk::SubpassDescription subpass = vk::SubpassDescription()
    .setPipelineBindPoint(vk::PipelineBindPoint::eGraphics)
    .setColorAttachmentCount(1)
    .setPColorAttachments(&colorAttachmentRef)
    .setPDepthStencilAttachment(&depthAttachmentRef);

  vk::SubpassDependency dependency = vk::SubpassDependency()
    .setDstSubpass(0)
    .setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite)
    .setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests)
    .setSrcSubpass(VK_SUBPASS_EXTERNAL)
    .setSrcAccessMask(vk::AccessFlagBits::eNone)
    .setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests);

  std::array<vk::AttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
  vk::RenderPassCreateInfo renderPassInfo = vk::RenderPassCreateInfo()
    .setAttachments(attachments)
    .setSubpassCount(1)
    .setPSubpasses(&subpass)
    .setDependencyCount(1)
    .setPDependencies(&dependency);

  auto renderPass = _device.createRenderPass(renderPassInfo);
  return renderPass;
}

}  // namespace engine

