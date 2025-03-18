#pragma once

#include "ve.hpp"

#include "input/application_input.hpp"
#include "input/mouse_input.hpp"
#include "input/imgui_input.hpp"
#include "camera.hpp"

struct Light
{
    alignas(16) glm::vec4 dirOrPos;
    alignas(16) glm::vec4 color; // rgb = color, a = intensity
};

struct LightsUniform
{
    Light lights[3];
    alignas(16) glm::vec4 ambiantColor; // rgb = color, a = intensity
};

struct CameraUniform
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 camPos;
};

struct ParametersUniform
{
    float time;
    float exposure;
};

struct SetLayouts
{
    SetLayouts()
        : mainLayout{ VK_NULL_HANDLE }
    {}
    void destroy(vk::Device &device)
    {
        device.destroyDescriptorSetLayout(mainLayout);
        mainLayout = VK_NULL_HANDLE;
    }

    vk::DescriptorSetLayout mainLayout;
};

struct PipelineLayouts
{
    PipelineLayouts()
        : mainLayout{ VK_NULL_HANDLE }
    {}
    void destroy(vk::Device &device)
    {
        device.destroyPipelineLayout(mainLayout);
        mainLayout = VK_NULL_HANDLE;
    }

    vk::PipelineLayout mainLayout;
};

struct Pipelines
{
    Pipelines()
        : ocean{ VK_NULL_HANDLE }
        , skybox{ VK_NULL_HANDLE }
    {}
    void destroy(vk::Device &device)
    {
        device.destroyPipeline(ocean);
        device.destroyPipeline(skybox);
        ocean = VK_NULL_HANDLE;
        skybox = VK_NULL_HANDLE;
    }

    vk::Pipeline ocean;
    vk::Pipeline skybox;
};

struct DescriptorSets
{
    DescriptorSets()
        : mainSets{}
    {}
    void destroy(vk::Device &device, vk::DescriptorPool &descriptorPool)
    {
        device.freeDescriptorSets(descriptorPool, mainSets);
        mainSets.clear();
    }

    std::vector<vk::DescriptorSet> mainSets;
};

class Application
{
public:
    Application(Framework &framework);

    void run();

private:
    Framework &m_framework;

    std::unique_ptr<InputManager> m_inputManager;
    enum InputID
    {
        MOUSE_INPUT, APP_INPUT, IMGUI_INPUT
    };

    Camera camera;

    void createBuffers();
    void createSetLayouts();
    void createPipelineLayouts();
    void createPipelines();
    void createDescriptorSets();

    void moveCamera(float dt);
    void updateUIFrame();
    void cleanUp();

    bool m_showUI = true;
    bool m_showPanelLights = false;
    bool m_showPanelParam = false;

    // Buffers
    std::unique_ptr<Buffer> m_cameraBuffer;
    std::unique_ptr<Buffer> m_lightsBuffer;
    std::unique_ptr<Buffer> m_paramBuffer;

    // Uniforms
    ParametersUniform m_param;
    LightsUniform m_lights;
    std::array<float, 3> m_lightLongitudes;
    std::array<float, 3> m_lightLatitudes;

    // Vulkan sets and pipelines
    SetLayouts m_setLayouts;
    PipelineLayouts m_pipelineLayouts;
    Pipelines m_pipelines;
    DescriptorSets m_descriptorSets;
};
