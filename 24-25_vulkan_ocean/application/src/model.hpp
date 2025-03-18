#pragma once

#include "ve.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

template <class Vtx>
class Model
{
public:
    Model(
        VulkanBase &base,
        std::vector<Vtx> &vertices,
        std::vector<uint32_t> &indices);
    ~Model();

    Model(const Model &) = delete;
    Model &operator=(const Model &) = delete;

    void bind(vk::CommandBuffer commandBuffer);
    void draw(vk::CommandBuffer commandBuffer);

protected:
    VulkanBase &m_base;

    Model(VulkanBase &base);

    void createVertexBuffer(const std::vector<Vtx> &vertices);
    void createIndexBuffer(const std::vector<uint32_t> &indices);

    std::unique_ptr<Buffer> m_vertexBuffer;
    std::unique_ptr<Buffer> m_indexBuffer;
};

class SkyboxModel : public Model<glm::vec3>
{
public:
    SkyboxModel(VulkanBase &base);
};

struct VertexUV {
    glm::vec3 pos;
    glm::vec2 texCoord;
};

class PlaneModel : public Model<VertexUV>
{
public:
    PlaneModel(VulkanBase &base, float size, int divisionCount);
};

class FullscreenModel : public Model<VertexUV>
{
public:
    FullscreenModel(VulkanBase &base);
};

struct SimpleVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 tangent;
    glm::vec2 texCoord;

    bool operator==(const SimpleVertex &other) const
    {
        return
            pos == other.pos &&
            normal == other.normal &&
            tangent == other.tangent &&
            texCoord == other.texCoord;
    }
};

class SimpleModel : public Model<SimpleVertex>
{
public:
    SimpleModel(
        VulkanBase &base,
        const std::string &filepath);
};

namespace std
{
    template<> struct hash<SimpleVertex> {
        size_t operator()(SimpleVertex const &vertex) const
        {
            size_t seed = 0;
            seed ^= hash<glm::vec3>()(vertex.pos) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash<glm::vec2>()(vertex.texCoord) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash<glm::vec3>()(vertex.normal) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= hash<glm::vec3>()(vertex.tangent) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
}

template<class Vtx>
Model<Vtx>::Model(VulkanBase &base, std::vector<Vtx> &vertices, std::vector<uint32_t> &indices)
    : m_base{ base }
{
    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}

template<class Vtx>
Model<Vtx>::~Model()
{
}

template<class Vtx>
void Model<Vtx>::bind(vk::CommandBuffer commandBuffer)
{
    commandBuffer.bindVertexBuffers(0, { m_vertexBuffer->getBuffer() }, { 0 });
    commandBuffer.bindIndexBuffer(m_indexBuffer->getBuffer(), 0, vk::IndexType::eUint32);
}

template<class Vtx>
void Model<Vtx>::draw(vk::CommandBuffer commandBuffer)
{
    commandBuffer.drawIndexed(
        static_cast<uint32_t>(m_indexBuffer->getElementCount()),
        1, 0, 0, 0);
}

template<class Vtx>
Model<Vtx>::Model(VulkanBase &base)
    : m_base{ base }
{
}

template<class Vtx>
void Model<Vtx>::createVertexBuffer(const std::vector<Vtx> &vertices)
{
    uint32_t vertexCount = static_cast<uint32_t>(vertices.size());
    assert(vertexCount >= 3 && "Vertex count must be at least 3");

    uint32_t vertexSize = sizeof(Vtx);

    vk::PhysicalDeviceMemoryProperties memoryProperties = m_base.getMemoryProperties();

    Buffer stagingBuffer{
        m_base.getDevice(),
        memoryProperties,
        vertexCount,
        vertexSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)vertices.data());

    m_vertexBuffer = std::make_unique<Buffer>(
        m_base.getDevice(),
        memoryProperties,
        vertexCount,
        vertexSize,
        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::CommandBuffer commandBuffer = tools::beginSingleTimeCommands(m_base);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = stagingBuffer.getBufferSize();
    commandBuffer.copyBuffer(stagingBuffer.getBuffer(), m_vertexBuffer->getBuffer(), 1, &copyRegion);

    tools::endSingleTimeCommands(m_base, commandBuffer);
}

template<class Vtx>
void Model<Vtx>::createIndexBuffer(const std::vector<uint32_t> &indices)
{
    uint32_t indexCount = static_cast<uint32_t>(indices.size());
    if (indexCount == 0) return;

    uint32_t indexSize = sizeof(indices[0]);

    Buffer stagingBuffer{
        m_base.getDevice(),
        m_base.getMemoryProperties(),
        indexCount,
        indexSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)indices.data());

    m_indexBuffer = std::make_unique<Buffer>(
        m_base.getDevice(),
        m_base.getMemoryProperties(),
        indexCount,
        indexSize,
        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
        vk::MemoryPropertyFlagBits::eDeviceLocal);

    vk::CommandBuffer commandBuffer = tools::beginSingleTimeCommands(m_base);

    vk::BufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = stagingBuffer.getBufferSize();
    commandBuffer.copyBuffer(stagingBuffer.getBuffer(), m_indexBuffer->getBuffer(), 1, &copyRegion);

    tools::endSingleTimeCommands(m_base, commandBuffer);
}