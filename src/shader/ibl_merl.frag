#version 450 core

in vec3 WorldPos;
in vec3 Normal;

layout (location = 0) out vec4 FragColor;

uniform sampler3D brdfLUT;
uniform samplerCube envMap;
uniform vec3 viewPos;
uniform int frameCount;

const float PI = 3.14159265359;
const int SAMPLES_PER_FRAME = 4;

uint hash(uint x) {
    x ^= x >> 16;
    x *= 0x45d9f3bu;
    x ^= x >> 16;
    x *= 0x45d9f3bu;
    x ^= x >> 16;
    return x;
}

float randomFloat(vec2 seed) {
    uint h = hash(uint(seed.x) * 1973u + uint(seed.y) * 9277u + uint(frameCount) * 26699u);
    return float(h) / 4294967295.0;
}

vec3 sampleCosineHemisphere(vec2 xi, vec3 N) {
    float phi = 2.0 * PI * xi.x;
    float cosTheta = sqrt(xi.y);
    float sinTheta = sqrt(1.0 - xi.y);

    vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    return normalize(tangent * cos(phi) * sinTheta +
                     bitangent * sin(phi) * sinTheta +
                     N * cosTheta);
}

vec3 lookupBrdf(vec3 N, vec3 V, vec3 L) {
    vec3 H = normalize(V + L);

    float NdotH = max(dot(N, H), 0.0);
    float HdotL = max(dot(H, L), 0.0);

    float theta_h = acos(clamp(NdotH, 0.0, 1.0));
    float theta_d = acos(clamp(HdotL, 0.0, 1.0));

    vec3 T;
    if (NdotH > 0.999) {
        vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, H));
    } else {
        T = normalize(N - NdotH * H);
    }
    vec3 B = cross(H, T);

    float phi_d = atan(dot(L, B), dot(L, T));
    if (phi_d < 0.0)
        phi_d += PI;

    float u = phi_d / PI;
    float v = theta_d / (PI * 0.5);
    float w = sqrt(clamp(theta_h / (PI * 0.5), 0.0, 1.0));

    return texture(brdfLUT, vec3(u, v, w)).rgb;
}

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - WorldPos);

    //if (!gl_FrontFacing) {
    //    N = -N;
    //}

    vec3 Lo = vec3(0.0);
    float validSamples = 0.0;

    vec2 pixelSeed = vec2(gl_FragCoord.xy);

    for (int i = 0; i < SAMPLES_PER_FRAME; ++i) {
        vec2 xi;
        xi.x = randomFloat(pixelSeed + vec2(float(i) * 17.3, float(frameCount) * 31.7));
        xi.y = randomFloat(pixelSeed + vec2(float(i) * 53.1, float(frameCount) * 97.3));

        vec3 L = sampleCosineHemisphere(xi, N);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL <= 0.0) continue;

        vec3 brdf = lookupBrdf(N, V, L);

        vec3 envRadiance = textureLod(envMap, L, 0.0).rgb;

        Lo += brdf * envRadiance;
        validSamples += 1.0;
    }

    if (validSamples > 0.0) {
        Lo /= validSamples;
        Lo *= PI;
    }
    FragColor = vec4(Lo, 1.0);
}
