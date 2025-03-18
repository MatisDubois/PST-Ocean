#include "application.hpp"
#include "model.hpp"
#include "stb_image.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define DEG_TO_RAD 0.01745329251994329576923690768489f
#define TAU 6.283185307179586476925286766559f

Application::Application(Framework &framework)
    : m_framework{ framework }
    , m_lightLatitudes{ 0.f, 0.f, 0.f }
    , m_lightLongitudes{ 0.f, 0.f, 0.f }
{
    m_param.time = 0.f;
    m_param.exposure = 1.f;

    m_lightLongitudes[0] = 90.f;
    m_lightLongitudes[1] = -90.f;
    m_lightLongitudes[2] = 90.f;

    m_lightLatitudes[0] = 80.f;
    m_lightLatitudes[1] = 25.f;
    m_lightLatitudes[2] = 25.f;

    m_lights.lights[0].color = { 255.f / 255.f, 255.f / 255.f, 255.f / 255.f, 3.f };
    m_lights.lights[1].color = { 250.f / 255.f, 161.f / 255.f, 161.f / 255.f, 1.f };
    m_lights.lights[2].color = { 255.f / 255.f, 236.f / 255.f, 122.f / 255.f, 3.f };
    m_lights.ambiantColor = { 3.f / 255.f,  40.f / 255.f,  84.f / 255.f, 0.1f };
    m_lights.lights[0].dirOrPos = { 1.f, 1.f, 1.f, 1.f };
    m_lights.lights[1].dirOrPos = { 1.f, 1.f, 1.f, 1.f };
    m_lights.lights[2].dirOrPos = { 1.f, 1.f, 1.f, 1.f };
}

void Application::run()
{
    vk::PhysicalDeviceProperties physicalProperties = m_framework.getProperties();
    vk::PhysicalDeviceMemoryProperties memoryProperties = m_framework.getMemoryProperties();
    vk::Device device = m_framework.getDevice();
    vk::CommandPool commandPool = m_framework.getCommandPool();
    vk::PipelineCache pipelineCache = m_framework.getPipelineCache();
    vk::DescriptorPool descriptorPool = m_framework.getDescriptorPool();

    Renderer &renderer = m_framework.getRenderer();
    vk::RenderPass renderPass = renderer.getRenderPass();
    Window &m_window = m_framework.getWindow();

    //==========================================================================
    // Sets and pipelines

    createBuffers();
    createSetLayouts();
    createPipelineLayouts();
    createPipelines();
    createDescriptorSets();

    //==========================================================================
    // Initialisation

    camera.setPerspectiveProjection(
        glm::radians(50.f), renderer.getAspectRatio(),
        0.1f, 100.0f);
    camera.setViewTarget(
        glm::vec3(0.f, 1.5f, 3.f),
        glm::vec3(0.f, 0.f, 0.f),
        glm::vec3(0.f, 1.f, 0.f));

    // Model
    SkyboxModel skyboxModel(m_framework.getVulkanBase());
    PlaneModel oceanModel(m_framework.getVulkanBase(), 100.f, 4096);
    //==========================================================================
    // Setup ImGui

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;

    io.Fonts->AddFontFromFileTTF("../../assets/font/Roboto-Medium.ttf", 16.0f);

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForVulkan(m_framework.getWindow().getSDL());
    ImGui_ImplVulkan_InitInfo guiInitInfo = {};
    guiInitInfo.Instance = m_framework.getInstance();
    guiInitInfo.PhysicalDevice = m_framework.getPhysicalDevice();
    guiInitInfo.Device = device;
    guiInitInfo.QueueFamily = m_framework.getGraphicsQueueFamilyIndex();
    guiInitInfo.Queue = m_framework.getGraphicsQueue();
    guiInitInfo.PipelineCache = m_framework.getPipelineCache();
    guiInitInfo.DescriptorPool = m_framework.getDescriptorPool();
    guiInitInfo.RenderPass = m_framework.getRenderer().getRenderPass();
    guiInitInfo.Subpass = 0;
    guiInitInfo.MinImageCount = Renderer::MAX_FRAMES_IN_FLIGHT;
    guiInitInfo.ImageCount = Renderer::MAX_FRAMES_IN_FLIGHT;
    guiInitInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    guiInitInfo.Allocator = nullptr;
    guiInitInfo.CheckVkResultFn = nullptr;
    ImGui_ImplVulkan_Init(&guiInitInfo);

    //==========================================================================
    // Boucle de rendu

    Timer *time = new Timer();
    time->start();

    // Input manager
    m_inputManager = std::make_unique<InputManager>();
    MouseInput *mouseInput = new MouseInput();

    m_inputManager->addInputGroup(MOUSE_INPUT, new MouseInput());
    m_inputManager->addInputGroup(APP_INPUT, new ApplicationInput());
    m_inputManager->addInputGroup(IMGUI_INPUT, new ImGuiInput());

    ApplicationInput *appInput = dynamic_cast<ApplicationInput *>(
        m_inputManager->getInputGroup(APP_INPUT));

    glm::mat4 modelMatrix = glm::mat4(1.f);

    int selectedMaterialID = 0;

    while (true)
    {
        time->update();
        float dt = time->getDelta();

        // Process events
        m_inputManager->processEvents();

        if (appInput->quitPressed) break;
        if (appInput->hideGuiPressed) m_showUI = !m_showUI;

        moveCamera(dt);

        modelMatrix = glm::mat4(1.f);

        for (int i = 0; i < 3; i++)
        {
            glm::vec3 lightDirection = glm::vec3(0.f, 0.f, 1.f);
            float latitude = m_lightLatitudes[i] * DEG_TO_RAD;
            float longitude = m_lightLongitudes[i] * DEG_TO_RAD;
            lightDirection = glm::rotateX(lightDirection, -latitude);
            lightDirection = glm::rotateY(lightDirection, longitude);
            m_lights.lights[i].dirOrPos = { lightDirection.x, lightDirection.y, lightDirection.z, 1.f };
        }

        m_param.time = time->getElapsed();

        // Check swapchain
        m_window.update();
        VkExtent2D extent = m_window.getExtent();
        if (extent.width < 8 || extent.height < 8 || m_window.isMinimized())
        {
            continue;
        }
        else if (m_window.isResized())
        {
            m_window.resetIsResized();
            renderer.recreateSwapchain(extent);

            camera.setPerspectiveProjection(
                glm::radians(50.f), renderer.getAspectRatio(),
                0.1f, 100.0f);
        }

        CameraUniform ubo{};
        ubo.view = camera.getView();
        ubo.proj = camera.getProjection();
        ubo.camPos = camera.getPosition();

        // Record command buffer
        vk::CommandBuffer commandBuffer = renderer.beginFrame();
        if (commandBuffer == nullptr) continue;

        uint32_t frameIndex = renderer.getFrameIndex();

        m_cameraBuffer->writeElementToBuffer(&ubo, frameIndex);
        m_lightsBuffer->writeElementToBuffer(&m_lights, frameIndex);
        m_paramBuffer->writeElementToBuffer(&m_param, frameIndex);

        std::array<vk::DescriptorSet, 1> sets = {
            m_descriptorSets.mainSets[frameIndex],
        };

        // Render pass
        renderer.beginRenderPass();

        // ocean model
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayouts.mainLayout, 0, sets, nullptr);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.ocean);

        commandBuffer.pushConstants(
            m_pipelineLayouts.mainLayout,
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eFragment,
            0, sizeof(glm::mat4), &modelMatrix);

        oceanModel.bind(commandBuffer);
        oceanModel.draw(commandBuffer);

        // Skybox
        glm::mat4 skyboxModelMatrix = glm::mat4(1.f);
        skyboxModelMatrix[3].x = camera.getPosition().x;
        skyboxModelMatrix[3].y = camera.getPosition().y;
        skyboxModelMatrix[3].z = camera.getPosition().z;
        commandBuffer.pushConstants(
            m_pipelineLayouts.mainLayout,
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eFragment,
            0, sizeof(glm::mat4), &skyboxModelMatrix);

        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayouts.mainLayout, 0, sets, nullptr);
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipelines.skybox);
        skyboxModel.bind(commandBuffer);
        skyboxModel.draw(commandBuffer);

        // UI
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        updateUIFrame();
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

        renderer.endRenderPass();
        renderer.endFrame();
    }
    device.waitIdle();

    //==========================================================================
    // Free memory

    cleanUp();
}

void Application::createBuffers()
{
    vk::Device device = m_framework.getDevice();
    vk::PhysicalDeviceProperties physicalProperties = m_framework.getProperties();
    vk::PhysicalDeviceMemoryProperties memoryProperties = m_framework.getMemoryProperties();
    VkDeviceSize minUniformBufferOffsetAlignment =
        physicalProperties.limits.minUniformBufferOffsetAlignment;

    m_cameraBuffer = std::make_unique<Buffer>(
        device,
        memoryProperties,
        2,
        sizeof(CameraUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent,
        minUniformBufferOffsetAlignment
        );
    m_cameraBuffer->map();

    m_lightsBuffer = std::make_unique<Buffer>(
        device,
        memoryProperties,
        2,
        sizeof(LightsUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent,
        minUniformBufferOffsetAlignment
        );
    m_lightsBuffer->map();

    m_paramBuffer = std::make_unique<Buffer>(
        device,
        memoryProperties,
        2,
        sizeof(ParametersUniform),
        vk::BufferUsageFlagBits::eUniformBuffer,
        vk::MemoryPropertyFlagBits::eHostVisible |
        vk::MemoryPropertyFlagBits::eHostCoherent,
        minUniformBufferOffsetAlignment
        );
    m_paramBuffer->map();

}

void Application::createSetLayouts()
{
    vk::Device device = m_framework.getDevice();

    m_setLayouts.mainLayout =
        DescriptorSetLayoutBuilder()
        // [Binding 0] Camera
        .addBinding(
            0, vk::DescriptorType::eUniformBuffer,
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eFragment
        )
        // [Binding 1] Parameters
        .addBinding(
            1, vk::DescriptorType::eUniformBuffer,
            vk::ShaderStageFlagBits::eVertex |
            vk::ShaderStageFlagBits::eFragment
        )
        // [Binding 2] Lights
        .addBinding(
            2, vk::DescriptorType::eUniformBuffer,
            vk::ShaderStageFlagBits::eFragment
        )
        .build(device);

}

void Application::createPipelineLayouts()
{
    vk::Device device = m_framework.getDevice();

    m_pipelineLayouts.mainLayout =
        PipelineLayoutBuilder()
        // [Set 0] Camera, Parameters, Lights
        .addDescriptorSetLayout(m_setLayouts.mainLayout)
        // [Push constant] Model matrix
        .addPushConstantRange(
            vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment,
            0, sizeof(glm::mat4))
        .build(device);
}

void Application::createPipelines()
{
    vk::Device device = m_framework.getDevice();
    vk::PipelineCache pipelineCache = m_framework.getPipelineCache();
    Renderer &renderer = m_framework.getRenderer();
    vk::RenderPass renderPass = renderer.getRenderPass();

    // Ocean pipeline
    {
        vk::PipelineShaderStageCreateInfo vertStage = tools::loadShader(
            device, "../shaders/ocean.vert.spv",
            vk::ShaderStageFlagBits::eVertex);
        vk::PipelineShaderStageCreateInfo fragStage = tools::loadShader(
            device, "../shaders/ocean.frag.spv",
            vk::ShaderStageFlagBits::eFragment);

        m_pipelines.ocean = PipelineBuilder(
            m_pipelineLayouts.mainLayout,
            renderPass, pipelineCache
        )
            .addVertexBindingDescription(
                0, sizeof(VertexUV),
                vk::VertexInputRate::eVertex
            )
            .addVertexAttributeDescription(
                0, 0, vk::Format::eR32G32B32Sfloat,
                offsetof(VertexUV, pos)
            )
            .addVertexAttributeDescription(
                1, 0, vk::Format::eR32G32Sfloat,
                offsetof(VertexUV, texCoord)
            )
            .addShaderStage(vertStage)
            .addShaderStage(fragStage)
            .build(device);

        device.destroyShaderModule(vertStage.module);
        device.destroyShaderModule(fragStage.module);
    }

    // Skybox pipeline
    {
        vk::PipelineShaderStageCreateInfo vertStage = tools::loadShader(
            device, "../shaders/skybox.vert.spv",
            vk::ShaderStageFlagBits::eVertex);
        vk::PipelineShaderStageCreateInfo fragStage = tools::loadShader(
            device, "../shaders/skybox.frag.spv",
            vk::ShaderStageFlagBits::eFragment);

        m_pipelines.skybox = PipelineBuilder(
            m_pipelineLayouts.mainLayout,
            renderPass, pipelineCache
        )
            .addVertexBindingDescription(
                0, sizeof(glm::vec3),
                vk::VertexInputRate::eVertex
            )
            .addVertexAttributeDescription(
                0, 0, vk::Format::eR32G32B32Sfloat,
                0
            )
            .addShaderStage(vertStage)
            .addShaderStage(fragStage)
            .build(device);

        device.destroyShaderModule(vertStage.module);
        device.destroyShaderModule(fragStage.module);
    }
}

void Application::createDescriptorSets()
{
    assert(
        m_cameraBuffer && m_paramBuffer && m_lightsBuffer &&
        "The buffers must be loaded first"
    );

    vk::Device device = m_framework.getDevice();
    vk::DescriptorPool descriptorPool = m_framework.getDescriptorPool();

    // Main Sets

    m_descriptorSets.mainSets =
        DescriptorSetBuilder()
        .addLayout(m_setLayouts.mainLayout)
        .addLayout(m_setLayouts.mainLayout)
        .build(device, descriptorPool);

    for (int i = 0; i < m_descriptorSets.mainSets.size(); i++)
    {
        vk::DescriptorBufferInfo cameraInfo = m_cameraBuffer->getDescriptorInfo();
        vk::DescriptorBufferInfo paramInfo = m_paramBuffer->getDescriptorInfo();
        vk::DescriptorBufferInfo lightsInfo = m_lightsBuffer->getDescriptorInfo();

        cameraInfo.offset = m_cameraBuffer->getAlignmentSize() * i;
        paramInfo.offset = m_paramBuffer->getAlignmentSize() * i;
        lightsInfo.offset = m_lightsBuffer->getAlignmentSize() * i;

        DescriptorSetUpdater()
            .beginDescriptorSet(m_descriptorSets.mainSets[i])
            .addBuffer(0, vk::DescriptorType::eUniformBuffer, &cameraInfo)
            .addBuffer(1, vk::DescriptorType::eUniformBuffer, &paramInfo)
            .addBuffer(2, vk::DescriptorType::eUniformBuffer, &lightsInfo)
            .update(device);
    }
}

void Application::moveCamera(float dt)
{
    ImGuiIO &io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;

    MouseInput *mouseInput = dynamic_cast<MouseInput *>(
        m_inputManager->getInputGroup(MOUSE_INPUT));

    glm::vec3 position = camera.getPosition();
    glm::vec3 viewDirection = camera.getViewDirection();
    const glm::vec3 upDir{0.f, 1.f, 0.f};
    const glm::vec3 rightDir{ glm::cross(upDir, -viewDirection) };
    vk::Extent2D extent = m_framework.getWindow().getExtent();
    float windowSize = static_cast<float>(
        extent.width > extent.height ? extent.width : extent.height);

    glm::vec2 delta = 1.f / windowSize * mouseInput->deltaPos;

    if (mouseInput->leftDown)
    {
        position = glm::rotate(position, -4.0f * delta.y, rightDir);
        position = glm::rotate(position, -4.0f * delta.x, upDir);
    }
    int wheel = mouseInput->wheel;
    if (wheel != 0)
    {
        float norm = glm::length(position);
        if (wheel > 0 && norm > 1.3f) position *= 0.9f;
        else if (wheel < 0 && norm < 20.f) position /= 0.9f;
    }

    camera.setViewTarget(position, glm::vec3(0.f, 0.f, 0.f), upDir);
}

void Application::updateUIFrame()
{
    if (m_showUI == false) return;

    ImGuiIO &io = ImGui::GetIO();


    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("View"))
    {
        ImGui::MenuItem("Param panel", NULL, &m_showPanelParam);
        ImGui::MenuItem("Lights panel", NULL, &m_showPanelLights);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();

    if (m_showPanelParam)
    {
        ImGui::Begin("Param Panel", &m_showPanelParam, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderFloat("Exposure", &m_param.exposure, 1.f, 15.f, "%.1f");
        ImGui::End();
    }

    if (m_showPanelLights)
    {
        ImGui::Begin("Lights Panel", &m_showPanelLights, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::ColorEdit3("Ambiant Color", (float *)&(m_lights.ambiantColor));
        ImGui::SliderFloat("Ambiant Intensity", &(m_lights.ambiantColor[3]), 0.f, 2.f, "%.1f");

        ImGui::SeparatorText("Directionnal lights");

        if (ImGui::CollapsingHeader("Light 0"))
        {
            ImGui::ColorEdit3("Color 0", (float *)&(m_lights.lights[0].color));
            ImGui::SliderFloat("Intensity 0", &(m_lights.lights[0].color[3]), 0.f, 5.f, "%.1f");
            ImGui::SliderFloat("Longitude 0", &(m_lightLongitudes[0]), -180.f, 180.f);
            ImGui::SliderFloat("Latitude 0", &(m_lightLatitudes[0]), -90.f, 90.f);
        }
        if (ImGui::CollapsingHeader("Light 1"))
        {
            ImGui::ColorEdit3("Color 1", (float *)&(m_lights.lights[1].color));
            ImGui::SliderFloat("Intensity 1", &(m_lights.lights[1].color[3]), 0.f, 5.f, "%.1f");
            ImGui::SliderFloat("Longitude 1", &(m_lightLongitudes[1]), -180.f, 180.f);
            ImGui::SliderFloat("Latitude 1", &(m_lightLatitudes[1]), -90.f, 90.f);
        }
        if (ImGui::CollapsingHeader("Light 2"))
        {
            ImGui::ColorEdit3("Color 2", (float *)&(m_lights.lights[2].color));
            ImGui::SliderFloat("Intensity 2", &(m_lights.lights[2].color[3]), 0.f, 5.f, "%.1f");
            ImGui::SliderFloat("Longitude 2", &(m_lightLongitudes[2]), -180.f, 180.f);
            ImGui::SliderFloat("Latitude 2", &(m_lightLatitudes[2]), -90.f, 90.f);
        }

        ImGui::End();
    }
}

void Application::cleanUp()
{
    vk::Device device = m_framework.getDevice();
    vk::DescriptorPool descriptorPool = m_framework.getDescriptorPool();

    ImGui_ImplVulkan_Shutdown();

    m_descriptorSets.destroy(device, descriptorPool);
    m_pipelines.destroy(device);
    m_pipelineLayouts.destroy(device);
    m_setLayouts.destroy(device);

    m_cameraBuffer.reset(nullptr);
    m_lightsBuffer.reset(nullptr);
    m_paramBuffer.reset(nullptr);
}
