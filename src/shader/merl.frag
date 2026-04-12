#version 450 core

in vec3 WorldPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler3D brdfLUT;
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform float lightIntensity;
uniform vec3 viewPos;

const float PI = 3.14159265359;

void main()
{
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - WorldPos);
    vec3 L = normalize(lightPos - WorldPos);
    vec3 H = normalize(V + L);

    float NdotL = max(dot(N, L), 0.0);
    if (NdotL <= 0.0) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    }

    float theta_h = acos(clamp(dot(N, H), 0.0, 1.0));
    float theta_d = acos(clamp(dot(H, L), 0.0, 1.0));

    vec3 T;
    if (dot(N, H) > 0.999) {
        vec3 up = abs(N.y) < 0.999 ? vec3(0.0, 1.0, 0.0) : vec3(1.0, 0.0, 0.0);
        T = normalize(cross(up, H));
    } else {
        T = normalize(N - dot(N, H) * H);
    }
    vec3 B = cross(H, T);

    float phi_d = atan(dot(L, B), dot(L, T));
    if (phi_d < 0.0)
        phi_d += PI;

    float u = phi_d / PI;
    float v = theta_d / (PI * 0.5);
    float w = sqrt(clamp(theta_h / (PI * 0.5), 0.0, 1.0));

    vec3 brdf = texture(brdfLUT, vec3(u, v, w)).rgb;

    float distance = length(lightPos - WorldPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 irradiance = lightColor * lightIntensity * attenuation;

    vec3 Lo = brdf * irradiance * NdotL;

    vec3 color = Lo;
    color = color / (color + vec3(1.0));
    //color = color / (color + 0.187) * 1.035;
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
