#include "model.hpp"
#include <tiny_obj_loader.h>

FullscreenModel::FullscreenModel(VulkanBase &base)
    : Model{ base }
{
    // Coordinates
    // 0 - 3
    // | \ |
    // 1 - 2

    // Vulkan NDC: from [-1,-1] top left corner to [1,1] bottom right corner.

    std::vector<VertexUV> vertices{};
    std::vector<uint32_t> indices{
        0, 1, 2, // Bottom left
        0, 2, 3  // Top right
    };
    VertexUV vertex{};
    vertex.pos = { -1.f, -1.f, 0.f }; vertex.texCoord = { 0.f, 0.f }; vertices.push_back(vertex);
    vertex.pos = { -1.f, +1.f, 0.f }; vertex.texCoord = { 0.f, 1.f }; vertices.push_back(vertex);
    vertex.pos = { +1.f, +1.f, 0.f }; vertex.texCoord = { 1.f, 1.f }; vertices.push_back(vertex);
    vertex.pos = { +1.f, -1.f, 0.f }; vertex.texCoord = { 1.f, 0.f }; vertices.push_back(vertex);

    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}

PlaneModel::PlaneModel(VulkanBase &base, float size, int divisionCount)
    : Model{ base }
{
    std::vector<VertexUV> vertices{};
    std::vector<uint32_t> indices{};

    VertexUV vertex{};

    float step = 1.0f / (float)divisionCount;
    for (int i = 0; i <= divisionCount; i++)
    {
        for (int j = 0; j <= divisionCount; j++)
        {
            float x = ((float)i - 0.5f * divisionCount) * size / (float)divisionCount;
            float y = ((float)j - 0.5f * divisionCount) * size / (float)divisionCount;
            vertex.pos = { x, 0.0f, y };
            vertex.texCoord = { i * step, j * step };
            vertices.push_back(vertex);
        }
    }

    for (int i = 0; i < divisionCount; i++)
    {
        for (int j = 0; j < divisionCount; j++)
        {
            indices.push_back(i * (divisionCount + 1) + j);
            indices.push_back((i + 1) * (divisionCount + 1) + j);
            indices.push_back(i * (divisionCount + 1) + (j + 1));

            indices.push_back((i + 1) * (divisionCount + 1) + j);
            indices.push_back((i + 1) * (divisionCount + 1) + (j + 1));
            indices.push_back(i * (divisionCount + 1) + (j + 1));
        }
    }

    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}


SkyboxModel::SkyboxModel(VulkanBase &base)
    : Model{ base }
{
    // Coordinates
    // 
    //   7--------6
    //  /|       /|
    // 4--------5 |
    // | |      | |
    // | 3------|-2
    // |/       |/
    // 0--------1

    std::vector<glm::vec3> vertices{};
    std::vector<uint32_t> indices{
        1, 2, 6, 6, 5, 1, // Right
        0, 4, 7, 7, 3, 0, // Left
        4, 5, 6, 6, 7, 4, // Top
        0, 3, 2, 2, 1, 0, // Bottom
        0, 1, 5, 5, 4, 0, // Back
        3, 7, 6, 6, 2, 3  // Front
    };
    glm::vec3 vertex{};
    vertex = { -1.0f, -1.0f, +1.0f }; vertices.push_back(vertex);
    vertex = { +1.0f, -1.0f, +1.0f }; vertices.push_back(vertex);
    vertex = { +1.0f, -1.0f, -1.0f }; vertices.push_back(vertex);
    vertex = { -1.0f, -1.0f, -1.0f }; vertices.push_back(vertex);
    vertex = { -1.0f, +1.0f, +1.0f }; vertices.push_back(vertex);
    vertex = { +1.0f, +1.0f, +1.0f }; vertices.push_back(vertex);
    vertex = { +1.0f, +1.0f, -1.0f }; vertices.push_back(vertex);
    vertex = { -1.0f, +1.0f, -1.0f }; vertices.push_back(vertex);

    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}

SimpleModel::SimpleModel(VulkanBase &base, const std::string &filepath)
    : Model{ base }
{
    std::vector<SimpleVertex> vertices{};
    std::vector<uint32_t> indices{};

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string errorString;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &errorString, filepath.c_str());
    if (success == false)
    {
        throw std::runtime_error(errorString);
    }

    std::unordered_map<SimpleVertex, uint32_t> uniqueVertices{};

    for (const auto &shape : shapes)
    {
        for (const auto &index : shape.mesh.indices)
        {
            SimpleVertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0 - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }
            else
            {
                vertex.normal = { 0.f, 1.f, 0.f };
            }

            vertex.tangent = { 0.f, 0.f, 0.f };

            if (uniqueVertices.count(vertex) == 0)
            {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }

            indices.push_back(uniqueVertices[vertex]);
        }
    }

    std::vector<glm::vec3> bitangents(vertices.size());

    for (int i = 0; i < indices.size(); i += 3)
    {
        assert(i + 2 < indices.size());
        uint32_t vertexIndices[3] = {
            indices[i + 0], indices[i + 1], indices[i + 2]
        };

        glm::vec3 v0 = vertices[vertexIndices[0]].pos;
        glm::vec3 v1 = vertices[vertexIndices[1]].pos;
        glm::vec3 v2 = vertices[vertexIndices[2]].pos;

        glm::vec2 uv0 = vertices[vertexIndices[0]].texCoord;
        glm::vec2 uv1 = vertices[vertexIndices[1]].texCoord;
        glm::vec2 uv2 = vertices[vertexIndices[2]].texCoord;

        glm::vec3 e1 = (v1 - v0);
        glm::vec3 e2 = (v2 - v0);

        float x1 = uv1.x - uv0.x;
        float x2 = uv2.x - uv0.x;
        float y1 = uv1.y - uv0.y;
        float y2 = uv2.y - uv0.y;

        float invDet = 1.0f / (x1 * y2 - x2 * y1);

        glm::vec3 tangent = glm::normalize(invDet * (y2 * e1 - y1 * e2));
        glm::vec3 bitangent = glm::normalize(invDet * (x1 * e2 - x2 * e1));

        for (int j = 0; j < 3; ++j)
        {
            vertices[vertexIndices[j]].tangent += tangent;
            bitangents[vertexIndices[j]] += bitangent;
        }
    }

    for (int i = 0; i < vertices.size(); i++)
    {
        const glm::vec3 &t = vertices[i].tangent;
        const glm::vec3 &b = bitangents[i];
        const glm::vec3 &n = vertices[i].normal;
        float sign = glm::dot(glm::cross(t, b), n) > 0.f ? -1.f : 1.f;
        sign = 1.0f;
        vertices[i].tangent = sign * glm::normalize(t);
    }

    std::cout << "Vertices = " << vertices.size() << std::endl;

    createVertexBuffer(vertices);
    createIndexBuffer(indices);
}