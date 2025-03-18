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

// In
layout(location = 0) in vec3 inWorldPos;
layout(location = 1) in vec3 inNormal;

// Out
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

//------------------------------------------------------------------------------
// Main

void main()
{
    vec3 ambiant = lightsUniform.ambiantColor.rgb
        * lightsUniform.ambiantColor.a;
    vec3 vecN = normalize(inNormal);
    vec3 vecV = normalize(ubo.camPos - inWorldPos);
    
    vec3 color = vec3(0.0001,0.0001,0.1);
    for (int i = 0; i < 3; i++)
    {
        float lightIntensity = lightsUniform.lights[i].color.a;
        vec3 lightColor = lightsUniform.lights[i].color.rgb;

        vec3 vecL = normalize(lightsUniform.lights[i].dirOrPos.xyz);
        vec3 vecH = normalize(vecL + vecV);
	    float NdotH = clamp(dot(vecN, vecH), 0.0, 1.0);

        color += lightColor * lightIntensity * pow(NdotH,20);
    }
    color += ambiant;
    color = max(color, vec3(0.0));
    
    // Tone-mapping and Gamma correction
    color = uncharted2Tonemap(color * param.exposure);
	color = pow(color, vec3(1.0f / 2.2f));

    outColor = vec4(color, 1.0);
}