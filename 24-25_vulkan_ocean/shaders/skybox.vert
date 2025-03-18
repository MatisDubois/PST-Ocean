#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

layout(set = 0, binding = 1) uniform ParamUniform
{
    float time;
    float exposure;
} param;

struct Light
{
    vec4 dirOrPos;
    vec4 color; // rgb = color, a = intensity
};

layout(set = 0, binding = 2) uniform LightsUniform
{
    Light lights[3];
    vec4 ambiantColor;
} lightsUniform;

layout(push_constant) uniform constants
{
	mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outWorldPos;

void main()
{
    vec4 worldPos = pushConstants.model * vec4(inPos, 1.0);
    outWorldPos = vec3(worldPos);
    gl_Position =  ubo.proj * ubo.view * worldPos;
    gl_Position.w = gl_Position.z;
}