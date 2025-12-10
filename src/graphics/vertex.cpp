#include "vertex.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescriptions() {
    std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(Vertex);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::vector<vk::VertexInputBindingDescription> Vertex::getBindingDescriptionsCpp() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0] = vk::VertexInputBindingDescription()
                                 .setBinding(0)
                                 .setStride(sizeof(Vertex))
                                 .setInputRate(vk::VertexInputRate::eVertex);
    return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});
    attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});
    attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
    attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

    return attributeDescriptions;
}


std::vector<vk::VertexInputAttributeDescription> Vertex::getAttributeDescriptionsCpp() {
    
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(0)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                                        .setOffset(offsetof(Vertex, position)));
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(1)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                                        .setOffset(offsetof(Vertex, color)));
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(2)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                                        .setOffset(offsetof(Vertex, normal)));
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(3)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32A32Sfloat)
                                        .setOffset(offsetof(Vertex, uv)));

    return attributeDescriptions;
}

bool Vertex::operator==(const Vertex &other) const {
    return position == other.position && color == other.color && normal == other.normal &&
            uv == other.uv;
}

namespace std {
    size_t hash<Vertex>::operator()(Vertex const &vertex) const {
        size_t seed = 0;
        lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
}  // namespace std
