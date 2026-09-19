#include "vertex.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>


namespace engine {

std::vector<vk::VertexInputBindingDescription> Vertex::getBindingDescriptions() {
    std::vector<vk::VertexInputBindingDescription> bindingDescriptions(1);
    bindingDescriptions[0] = vk::VertexInputBindingDescription()
                                 .setBinding(0)
                                 .setStride(sizeof(Vertex))
                                 .setInputRate(vk::VertexInputRate::eVertex);
    return bindingDescriptions;
}

std::vector<vk::VertexInputAttributeDescription> Vertex::getAttributeDescriptions() {
    
    std::vector<vk::VertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(0)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32Sfloat)
                                        .setOffset(offsetof(Vertex, position)));
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(1)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32Sfloat)
                                        .setOffset(offsetof(Vertex, color)));
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(2)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32B32Sfloat)
                                        .setOffset(offsetof(Vertex, normal)));
    attributeDescriptions.push_back(vk::VertexInputAttributeDescription()
                                        .setLocation(3)
                                        .setBinding(0)
                                        .setFormat(vk::Format::eR32G32Sfloat)
                                        .setOffset(offsetof(Vertex, uv)));

    return attributeDescriptions;
}

bool Vertex::operator==(const Vertex &other) const {
    return position == other.position && color == other.color && normal == other.normal &&
            uv == other.uv;
}

}

namespace std {
    size_t hash<engine::Vertex>::operator()(engine::Vertex const &vertex) const {
        size_t seed = 0;
        lve::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
        return seed;
    }
}  // namespace std
