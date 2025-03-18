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

layout(location = 0) in vec3 inWorldPos;

layout(location = 0) out vec4 outColor;

//------------------------------------------------------------------------------
// Tonemapping

vec3 uncharted2TonemapPartial(vec3 x)
{
	float A = 0.15f;
	float B = 0.50f;
	float C = 0.10f;
	float D = 0.20f;
	float E = 0.02f;
	float F = 0.30f;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

vec3 uncharted2Tonemap(vec3 c)
{
    vec3 w = vec3(11.2f);
    vec3 whiteScale = vec3(1.0f) / uncharted2TonemapPartial(w);
    return uncharted2TonemapPartial(c) * whiteScale;
}

vec3 jodieReinhardTonemap(vec3 c)
{
    float luminance = dot(c, vec3(0.2126f, 0.7152f, 0.0722f));
    vec3 tc = c / (c + 1.0f);
    return mix(c / (luminance + 1.0f), tc, tc);
}

void main()
{
    vec3 ambiantColor = lightsUniform.ambiantColor.rgb * lightsUniform.ambiantColor.a;
    vec3 groundColor = vec3(0.f);

    vec3 vecV = normalize(ubo.camPos - inWorldPos);

    float t = smoothstep(0.1f, 0.7f, vecV.y);
    vec3 color = mix(ambiantColor, groundColor, t);

    // Tone-mapping and Gamma correction
    color = uncharted2Tonemap(color * param.exposure);
	color = pow(color, vec3(1.0f / 5.2f));

    outColor = vec4(color, 1.0);
}