#include "jipu/buffer.h"
#include "jipu/command_buffer.h"
#include "jipu/command_encoder.h"
#include "jipu/device.h"
#include "jipu/driver.h"
#include "jipu/physical_device.h"
#include "jipu/pipeline.h"
#include "jipu/pipeline_layout.h"
#include "jipu/queue.h"
#include "jipu/surface.h"
#include "jipu/swapchain.h"

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <spdlog/spdlog.h>

using namespace jipu;


/*
#version 450

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec4 inColor;

layout(location = 0) out vec2 outUV;
layout(location = 1) out vec4 outColor;

layout(binding = 1) uniform UITransform {
    vec2 scale;
    vec2 translate;
} uiTransform;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outUV = inUV;
    outColor = inColor;
    gl_Position = vec4(inPos * uiTransform.scale + uiTransform.translate, 0.0, 1.0);
}
*/
static std::vector<uint32_t> vertexShaderSourceSpv = { 0x07230203, 0x00010000, 0x000d000b, 0x0000002b, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x000b000f, 0x00000000, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x0000000f, 0x00000011, 0x00000015, 0x00000018, 0x00030003, 0x00000002, 0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00040005, 0x00000009, 0x5574756f, 0x00000056, 0x00040005, 0x0000000b, 0x56556e69, 0x00000000, 0x00050005, 0x0000000f, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00040005, 0x00000011, 0x6f436e69, 0x00726f6c, 0x00060005, 0x00000013, 0x505f6c67, 0x65567265, 0x78657472, 0x00000000, 0x00060006, 0x00000013, 0x00000000, 0x505f6c67, 0x7469736f, 0x006e6f69, 0x00030005, 0x00000015, 0x00000000, 0x00040005, 0x00000018, 0x6f506e69, 0x00000073, 0x00050005, 0x0000001a, 0x72544955, 0x66736e61, 0x006d726f, 0x00050006, 0x0000001a, 0x00000000, 0x6c616373, 0x00000065, 0x00060006, 0x0000001a, 0x00000001, 0x6e617274, 0x74616c73, 0x00000065, 0x00050005, 0x0000001c, 0x72546975, 0x66736e61, 0x006d726f, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000b, 0x0000001e, 0x00000001, 0x00040047, 0x0000000f, 0x0000001e, 0x00000001, 0x00040047, 0x00000011, 0x0000001e, 0x00000002, 0x00050048, 0x00000013, 0x00000000, 0x0000000b, 0x00000000, 0x00030047, 0x00000013, 0x00000002, 0x00040047, 0x00000018, 0x0000001e, 0x00000000, 0x00050048, 0x0000001a, 0x00000000, 0x00000023, 0x00000000, 0x00050048, 0x0000001a, 0x00000001, 0x00000023, 0x00000008, 0x00030047, 0x0000001a, 0x00000002, 0x00040047, 0x0000001c, 0x00000022, 0x00000000, 0x00040047, 0x0000001c, 0x00000021, 0x00000001, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000002, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a, 0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00040017, 0x0000000d, 0x00000006, 0x00000004, 0x00040020, 0x0000000e, 0x00000003, 0x0000000d, 0x0004003b, 0x0000000e, 0x0000000f, 0x00000003, 0x00040020, 0x00000010, 0x00000001, 0x0000000d, 0x0004003b, 0x00000010, 0x00000011, 0x00000001, 0x0003001e, 0x00000013, 0x0000000d, 0x00040020, 0x00000014, 0x00000003, 0x00000013, 0x0004003b, 0x00000014, 0x00000015, 0x00000003, 0x00040015, 0x00000016, 0x00000020, 0x00000001, 0x0004002b, 0x00000016, 0x00000017, 0x00000000, 0x0004003b, 0x0000000a, 0x00000018, 0x00000001, 0x0004001e, 0x0000001a, 0x00000007, 0x00000007, 0x00040020, 0x0000001b, 0x00000002, 0x0000001a, 0x0004003b, 0x0000001b, 0x0000001c, 0x00000002, 0x00040020, 0x0000001d, 0x00000002, 0x00000007, 0x0004002b, 0x00000016, 0x00000021, 0x00000001, 0x0004002b, 0x00000006, 0x00000025, 0x00000000, 0x0004002b, 0x00000006, 0x00000026, 0x3f800000, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b, 0x0003003e, 0x00000009, 0x0000000c, 0x0004003d, 0x0000000d, 0x00000012, 0x00000011, 0x0003003e, 0x0000000f, 0x00000012, 0x0004003d, 0x00000007, 0x00000019, 0x00000018, 0x00050041, 0x0000001d, 0x0000001e, 0x0000001c, 0x00000017, 0x0004003d, 0x00000007, 0x0000001f, 0x0000001e, 0x00050085, 0x00000007, 0x00000020, 0x00000019, 0x0000001f, 0x00050041, 0x0000001d, 0x00000022, 0x0000001c, 0x00000021, 0x0004003d, 0x00000007, 0x00000023, 0x00000022, 0x00050081, 0x00000007, 0x00000024, 0x00000020, 0x00000023, 0x00050051, 0x00000006, 0x00000027, 0x00000024, 0x00000000, 0x00050051, 0x00000006, 0x00000028, 0x00000024, 0x00000001, 0x00070050, 0x0000000d, 0x00000029, 0x00000027, 0x00000028, 0x00000025, 0x00000026, 0x00050041, 0x0000000e, 0x0000002a, 0x00000015, 0x00000017, 0x0003003e, 0x0000002a, 0x00000029, 0x000100fd, 0x00010038 };

/*
#version 450

layout(binding = 0) uniform sampler2D fontSampler;

layout(location = 0) in vec2 inUV;
layout(location = 1) in vec4 inColor;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = inColor * texture(fontSampler, inUV);
}
*/
static std::vector<uint32_t> fragmentShaderSourceSpv = { 0x07230203, 0x00010000, 0x000d000b, 0x00000018, 0x00000000, 0x00020011, 0x00000001, 0x0006000b, 0x00000001, 0x4c534c47, 0x6474732e, 0x3035342e, 0x00000000, 0x0003000e, 0x00000000, 0x00000001, 0x0008000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, 0x00000009, 0x0000000b, 0x00000014, 0x00030010, 0x00000004, 0x00000007, 0x00030003, 0x00000002, 0x000001c2, 0x000a0004, 0x475f4c47, 0x4c474f4f, 0x70635f45, 0x74735f70, 0x5f656c79, 0x656e696c, 0x7269645f, 0x69746365, 0x00006576, 0x00080004, 0x475f4c47, 0x4c474f4f, 0x6e695f45, 0x64756c63, 0x69645f65, 0x74636572, 0x00657669, 0x00040005, 0x00000004, 0x6e69616d, 0x00000000, 0x00050005, 0x00000009, 0x4374756f, 0x726f6c6f, 0x00000000, 0x00040005, 0x0000000b, 0x6f436e69, 0x00726f6c, 0x00050005, 0x00000010, 0x746e6f66, 0x706d6153, 0x0072656c, 0x00040005, 0x00000014, 0x56556e69, 0x00000000, 0x00040047, 0x00000009, 0x0000001e, 0x00000000, 0x00040047, 0x0000000b, 0x0000001e, 0x00000001, 0x00040047, 0x00000010, 0x00000022, 0x00000000, 0x00040047, 0x00000010, 0x00000021, 0x00000000, 0x00040047, 0x00000014, 0x0000001e, 0x00000000, 0x00020013, 0x00000002, 0x00030021, 0x00000003, 0x00000002, 0x00030016, 0x00000006, 0x00000020, 0x00040017, 0x00000007, 0x00000006, 0x00000004, 0x00040020, 0x00000008, 0x00000003, 0x00000007, 0x0004003b, 0x00000008, 0x00000009, 0x00000003, 0x00040020, 0x0000000a, 0x00000001, 0x00000007, 0x0004003b, 0x0000000a, 0x0000000b, 0x00000001, 0x00090019, 0x0000000d, 0x00000006, 0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000, 0x0003001b, 0x0000000e, 0x0000000d, 0x00040020, 0x0000000f, 0x00000000, 0x0000000e, 0x0004003b, 0x0000000f, 0x00000010, 0x00000000, 0x00040017, 0x00000012, 0x00000006, 0x00000002, 0x00040020, 0x00000013, 0x00000001, 0x00000012, 0x0004003b, 0x00000013, 0x00000014, 0x00000001, 0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, 0x000200f8, 0x00000005, 0x0004003d, 0x00000007, 0x0000000c, 0x0000000b, 0x0004003d, 0x0000000e, 0x00000011, 0x00000010, 0x0004003d, 0x00000012, 0x00000015, 0x00000014, 0x00050057, 0x00000007, 0x00000016, 0x00000011, 0x00000015, 0x00050085, 0x00000007, 0x00000017, 0x0000000c, 0x00000016, 0x0003003e, 0x00000009, 0x00000017, 0x000100fd, 0x00010038 };


class TriangleSample
{
public:
    TriangleSample();
    ~TriangleSample();

private:
    void updateUniformBuffer();

private:
    void createDevier();
    void getPhysicalDevices();
    void createSurface();
    void createDevice();
    void createSwapchain();
    void createCommandBuffer();
    void createQueue();
    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffer();
    void createBindingGroupLayout();
    void createBindingGroup();
    void createRenderPipeline();

private:
    std::unique_ptr<Driver> m_driver = nullptr;
    std::vector<std::unique_ptr<PhysicalDevice>> m_physicalDevices{};
    std::unique_ptr<Surface> m_surface = nullptr;
    std::unique_ptr<Device> m_device = nullptr;
    std::unique_ptr<Swapchain> m_swapchain = nullptr;
    std::unique_ptr<CommandBuffer> m_commandBuffer = nullptr;
    std::unique_ptr<Queue> m_queue = nullptr;
    std::unique_ptr<Buffer> m_vertexBuffer = nullptr;
    std::unique_ptr<Buffer> m_indexBuffer = nullptr;
    std::unique_ptr<Buffer> m_uniformBuffer = nullptr;
    std::unique_ptr<BindingGroupLayout> m_bindingGroupLayout = nullptr;
    std::unique_ptr<BindingGroup> m_bindingGroup = nullptr;
    std::unique_ptr<PipelineLayout> m_renderPipelineLayout = nullptr;
    std::unique_ptr<RenderPipeline> m_renderPipeline = nullptr;

    struct MVP
    {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct UBO
    {
        MVP mvp;
    } m_ubo;

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 color;
    };

    std::vector<uint16_t> m_indices{ 0, 1, 2 };
    std::vector<Vertex>
        m_vertices{
            { { 0.0, -500, 0.0 }, { 1.0, 0.0, 0.0 } },
            { { -500, 500, 0.0 }, { 0.0, 1.0, 0.0 } },
            { { 500, 500, 0.0 }, { 0.0, 0.0, 1.0 } },
        };

    uint32_t m_sampleCount = 1;
};

TriangleSample::TriangleSample()
{
    createDevier();
    getPhysicalDevices();
    createSurface();
    createDevice();
    createSwapchain();
    createCommandBuffer();
    createQueue();

    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffer();
    createBindingGroupLayout();
    createBindingGroup();
    createRenderPipeline();
}

TriangleSample::~TriangleSample()
{
    m_renderPipeline.reset();
    m_renderPipelineLayout.reset();
    m_bindingGroup.reset();
    m_bindingGroupLayout.reset();
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_uniformBuffer.reset();
    m_queue.reset();
    m_commandBuffer.reset();
    m_swapchain.reset();
    m_device.reset();
    m_surface.reset();
    m_physicalDevices.clear();
    m_driver.reset();
}

void TriangleSample::updateUniformBuffer()
{
    m_ubo.mvp.model = glm::mat4(1.0f);
    m_ubo.mvp.view = glm::lookAt(glm::vec3(0.0f, 0.0f, 1000.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0f, 0.0));
    m_ubo.mvp.proj = glm::perspective(45.0f,
                                        m_swapchain->getWidth() / static_cast<float>(m_swapchain->getHeight()),
                                        0.1f,
                                        1000.0f);

    void* pointer = m_uniformBuffer->map(); // do not unmap.
    memcpy(pointer, &m_ubo, m_uniformBuffer->getSize());
}

void TriangleSample::createDevier()
{
    DriverDescriptor descriptor{};
    descriptor.type = DriverType::kVulkan;

    m_driver = Driver::create(descriptor);
}

void TriangleSample::getPhysicalDevices()
{
    m_physicalDevices = m_driver->getPhysicalDevices();
}

void TriangleSample::createDevice()
{
    SurfaceDescriptor descriptor{};
    descriptor.windowHandle = nullptr;

    m_surface = m_driver->createSurface(descriptor);
}

void TriangleSample::createSurface()
{
    // TODO: select suit device.
    PhysicalDevice* physicalDevice = m_physicalDevices[0].get();

    DeviceDescriptor descriptor;
    m_device = physicalDevice->createDevice(descriptor);
}

void TriangleSample::createSwapchain()
{
#if defined(__ANDROID__) || defined(ANDROID)
    TextureFormat textureFormat = TextureFormat::kRGBA_8888_UInt_Norm_SRGB;
#else
    TextureFormat textureFormat = TextureFormat::kBGRA_8888_UInt_Norm_SRGB;
#endif

    SwapchainDescriptor descriptor{};
    descriptor.width = 1280;
    descriptor.height = 720;
    descriptor.surface = m_surface.get();
    descriptor.textureFormat = textureFormat;
    descriptor.colorSpace = ColorSpace::kSRGBNonLinear;
    descriptor.presentMode = PresentMode::kFifo;

    m_swapchain = m_device->createSwapchain(descriptor);
}

void TriangleSample::createCommandBuffer()
{
    CommandBufferDescriptor descriptor{};
    descriptor.usage = CommandBufferUsage::kOneTime;

    m_commandBuffer = m_device->createCommandBuffer(descriptor);
}

void TriangleSample::createQueue()
{
    QueueDescriptor descriptor{};
    descriptor.flags = QueueFlagBits::kGraphics;

    m_queue = m_device->createQueue(descriptor);
}

void TriangleSample::createVertexBuffer()
{
    BufferDescriptor descriptor{};
    descriptor.size = m_vertices.size() * sizeof(Vertex);
    descriptor.usage = BufferUsageFlagBits::kVertex;

    m_vertexBuffer = m_device->createBuffer(descriptor);

    void* pointer = m_vertexBuffer->map();
    memcpy(pointer, m_vertices.data(), descriptor.size);
    m_vertexBuffer->unmap();
}

void TriangleSample::createIndexBuffer()
{
    BufferDescriptor descriptor{};
    descriptor.size = m_indices.size() * sizeof(uint16_t);
    descriptor.usage = BufferUsageFlagBits::kIndex;

    m_indexBuffer = m_device->createBuffer(descriptor);

    void* pointer = m_indexBuffer->map();
    memcpy(pointer, m_indices.data(), descriptor.size);
    m_indexBuffer->unmap();
}

void TriangleSample::createUniformBuffer()
{
    BufferDescriptor descriptor{};
    descriptor.size = sizeof(UBO);
    descriptor.usage = BufferUsageFlagBits::kUniform;

    m_uniformBuffer = m_device->createBuffer(descriptor);

    void* pointer = m_uniformBuffer->map();
    // memcpy(pointer, &m_ubo, descriptor.size);
    // m_uniformBuffer->unmap();
}

void TriangleSample::createBindingGroupLayout()
{
    BufferBindingLayout bufferLayout{};
    bufferLayout.index = 0;
    bufferLayout.stages = BindingStageFlagBits::kVertexStage;
    bufferLayout.type = BufferBindingType::kUniform;

    BindingGroupLayoutDescriptor descriptor{};
    descriptor.buffers = { bufferLayout };

    m_bindingGroupLayout = m_device->createBindingGroupLayout(descriptor);
}

void TriangleSample::createBindingGroup()
{
    BufferBinding bufferBinding{};
    bufferBinding.buffer = m_uniformBuffer.get();
    bufferBinding.index = 0;
    bufferBinding.offset = 0;
    bufferBinding.size = m_uniformBuffer->getSize();

    BindingGroupDescriptor descriptor{};
    descriptor.layout = { m_bindingGroupLayout.get() };
    descriptor.buffers = { bufferBinding };

    m_bindingGroup = m_device->createBindingGroup(descriptor);
}

void TriangleSample::createRenderPipeline()
{
    // render pipeline layout
    {
        PipelineLayoutDescriptor descriptor{};
        descriptor.layouts = { m_bindingGroupLayout.get() };

        m_renderPipelineLayout = m_device->createPipelineLayout(descriptor);
    }

    // input assembly stage
    InputAssemblyStage inputAssemblyStage{};
    {
        inputAssemblyStage.topology = PrimitiveTopology::kTriangleList;
    }

    // vertex shader module
    std::unique_ptr<ShaderModule> vertexShaderModule = nullptr;
    {
        ShaderModuleDescriptor descriptor{};
        descriptor.code = reinterpret_cast<const char*>(vertexShaderSourceSpv.data());
        descriptor.codeSize = static_cast<uint32_t>(vertexShaderSourceSpv.size() * 4);

        vertexShaderModule = m_device->createShaderModule(descriptor);
    }

    // vertex stage
    VertexStage vertexStage{};
    {
        VertexAttribute positionAttribute{};
        positionAttribute.format = VertexFormat::kSFLOATx3;
        positionAttribute.offset = offsetof(Vertex, pos);
        positionAttribute.location = 0;

        VertexAttribute colorAttribute{};
        colorAttribute.format = VertexFormat::kSFLOATx3;
        colorAttribute.offset = offsetof(Vertex, color);
        colorAttribute.location = 1;

        VertexInputLayout vertexInputLayout{};
        vertexInputLayout.mode = VertexMode::kVertex;
        vertexInputLayout.stride = sizeof(Vertex);
        vertexInputLayout.attributes = { positionAttribute, colorAttribute };

        vertexStage.entryPoint = "main";
        vertexStage.shaderModule = vertexShaderModule.get();
        vertexStage.layouts = { vertexInputLayout };
    }

    // rasterization
    RasterizationStage rasterizationStage{};
    {
        rasterizationStage.cullMode = CullMode::kNone;
        rasterizationStage.frontFace = FrontFace::kCounterClockwise;
        rasterizationStage.sampleCount = m_sampleCount;
    }

    // fragment shader module
    std::unique_ptr<ShaderModule> fragmentShaderModule = nullptr;
    {
        ShaderModuleDescriptor descriptor{};
        descriptor.code = reinterpret_cast<const char*>(fragmentShaderSourceSpv.data());
        descriptor.codeSize = static_cast<uint32_t>(fragmentShaderSourceSpv.size() * 4);

        fragmentShaderModule = m_device->createShaderModule(descriptor);
    }

    // fragment
    FragmentStage fragmentStage{};
    {
        FragmentStage::Target target{};
        target.format = m_swapchain->getTextureFormat();

        fragmentStage.targets = { target };
        fragmentStage.entryPoint = "main";
        fragmentStage.shaderModule = fragmentShaderModule.get();
    }

    // depth/stencil

    // render pipeline
    RenderPipelineDescriptor descriptor{};
    descriptor.inputAssembly = inputAssemblyStage;
    descriptor.vertex = vertexStage;
    descriptor.rasterization = rasterizationStage;
    descriptor.fragment = fragmentStage;
    descriptor.layout = m_renderPipelineLayout.get();

    m_renderPipeline = m_device->createRenderPipeline(descriptor);
}

#if defined(__ANDROID__) || defined(ANDROID)

// GameActivity's C/C++ code
#include <game-activity/GameActivity.cpp>
#include <game-text-input/gametextinput.cpp>

// // Glue from GameActivity to android_main()
// // Passing GameActivity event from main thread to app native thread.
extern "C"
{
#include <game-activity/native_app_glue/android_native_app_glue.c>
}

void android_main(struct android_app* app)
{
    jipu::SampleDescriptor descriptor{
        { 1000, 2000, "Triangle", app },
        ""
    };

    jipu::TriangleSample sample(descriptor);

    sample.exec();
}

#else

int main(int argc, char** argv)
{
    spdlog::set_level(spdlog::level::trace);

    TriangleSample sample;

    return 0;
}

#endif
