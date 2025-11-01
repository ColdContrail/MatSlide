#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

out vec4 FragColor;

const float PI = 3.14159265359;

// 法线分布函数 (Trowbridge-Reitz GGX)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// 几何函数 (Schlick GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// Fresnel方程 (Schlick近似)
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main()
{
    // 硬编码材质属性 - 蓝色橡胶
    vec3 albedo = vec3(0.1, 0.3, 0.8);      // 深蓝色
    float metallic = 1.0;                   // 非金属
    float roughness = 0.5;                  // 高粗糙度（橡胶表面）
    
    // 硬编码光源属性
    vec3 lightPos = vec3(2.0, 3.0, 1.0);    // 光源位置
    vec3 lightColor = vec3(1.0, 1.0, 1.0);  // 白光
    float lightIntensity = 100.0;             // 光源强度
    
    // 硬编码摄像机位置
    vec3 viewPos = vec3(0.0, 0.0, 3.0);

    // 输入数据
    vec3 N = normalize(Normal);
    vec3 V = normalize(viewPos - FragPos);
    vec3 L = normalize(lightPos - FragPos);
    vec3 H = normalize(V + L);
    
    // 计算距离衰减
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lightColor * lightIntensity * attenuation;

    // 计算基础反射率
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // 计算反射和折射比例
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    // 计算最终光照
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // 环境光照 (简化版本)
    vec3 ambient = vec3(0.03) * albedo;

    vec3 color = ambient + Lo;

    // HDR色调映射
    color = color / (color + vec3(1.0));
    // Gamma校正
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
