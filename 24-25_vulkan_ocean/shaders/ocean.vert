#version 450

layout(set = 0, binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
    vec3 camPos;
} ubo;

layout(set = 0, binding = 1) uniform ParamUniform
{
    float time;
    float exposure;
} param;

// In
layout(location = 0) in vec3 inPos;
layout(location = 1) in vec2 inUV0;

// Out
layout(location = 0) out vec3 outWorldPos;
layout(location = 1) out vec3 outNormal;

layout(push_constant) uniform constants
{
	mat4 model;
} pushConstants;

// Fonction pour une vague de Gerstner
vec3 gerstnerWave(vec2 position, vec2 direction, float steepness, float wavelength, float time, inout vec3 tangent, inout vec3 binormal) 
{
    float k = 2.0 * 3.14159265359 / wavelength; // Nombre d'onde (fréquence)
    float c = sqrt(9.8 / k) * 0.5; // Vitesse réduite pour un mouvement naturel
    vec2 d = normalize(direction); // Direction normalisée
    float f = k * (dot(d, position) - c * time); // Phase

    float a = steepness / k; // Amplitude ajustée

    // Déplacement du sommet
    vec3 displacement = vec3(
        d.x * (a * cos(f)), 
        a * sin(f),
        d.y * (a * cos(f))
    );

    // Mise à jour des tangentes pour la normale
    tangent += vec3(
        -d.x * d.x * (steepness * sin(f)),
        d.x * (steepness * cos(f)),
        -d.x * d.y * (steepness * sin(f))
    );

    binormal += vec3(
        -d.x * d.y * (steepness * sin(f)),
        d.y * (steepness * cos(f)),
        -d.y * d.y * (steepness * sin(f))
    );

    return displacement;
}

void main()
{
    vec4 locPos = pushConstants.model * vec4(inPos, 1.0);
    outWorldPos = locPos.xyz / locPos.w;

    float time = param.time;

    // Initialisation des tangentes pour la normale
    vec3 tangent = vec3(1.0, 0.0, 0.0);
    vec3 binormal = vec3(0.0, 0.0, 1.0);

    // Initialisation du déplacement des vagues
    vec3 waveDisplacement = vec3(0.0);

    // Ajout de plusieurs vagues avec vitesse ajustée
    waveDisplacement += gerstnerWave(outWorldPos.xz, vec2(1.0, 0.5), 0.2, 2.0, time, tangent, binormal);
    waveDisplacement += gerstnerWave(outWorldPos.xz, vec2(-0.7, 1.0), 0.15, 1.5, time, tangent, binormal);
    waveDisplacement += gerstnerWave(outWorldPos.xz, vec2(0.3, -0.8), 0.1, 1.2, time, tangent, binormal);
    waveDisplacement += gerstnerWave(outWorldPos.xz, vec2(-1.0, -0.2), 0.08, 1.0, time, tangent, binormal);

    // Appliquer le déplacement
    outWorldPos += waveDisplacement;

    // Calcul de la normale finale
    outNormal = normalize(cross(binormal, tangent));

    // Projection finale
    gl_Position = ubo.proj * ubo.view * vec4(outWorldPos, 1.0);
}
