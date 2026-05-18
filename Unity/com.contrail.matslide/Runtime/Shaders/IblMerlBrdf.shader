Shader "Custom/IblMerlBrdf"
{
    Properties
    {
        _BrdfLUT ("BRDF LUT", 3D) = "white" {}
        _EnvCube ("Environment Cube", CUBE) = "" {}
        _Roughness ("Roughness", Range(0.01, 1.0)) = 0.1
        _Exposure ("Exposure", Float) = 1.0
        _MaxSampleValue ("Max Sample Value", Float) = 10.0
        _LightPos ("Light Position", Vector) = (2, 3, 1, 0)
        _LightColor ("Light Color", Color) = (1, 1, 1, 1)
        _LightIntensity ("Light Intensity", Float) = 100.0
    }

    SubShader
    {
        Tags { "RenderType"="Opaque" "RenderPipeline"="UniversalPipeline" }
        LOD 300

        Pass
        {
            Name "ForwardLit"
            Tags { "LightMode"="UniversalForward" }

            HLSLPROGRAM
            #pragma vertex vert
            #pragma fragment frag

            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Core.hlsl"

            #define PI 3.14159265359
            #define DIFFUSE_SAMPLES  16
            #define SPECULAR_SAMPLES 16

            TEXTURE3D(_BrdfLUT);
            SAMPLER(sampler_BrdfLUT);
            TEXTURECUBE(_EnvCube);
            SAMPLER(sampler_EnvCube);

            float _Roughness;
            float _Exposure;
            float _MaxSampleValue;
            float4 _LightPos;
            float4 _LightColor;
            float _LightIntensity;

            struct Attributes
            {
                float4 positionOS : POSITION;
                float3 normalOS   : NORMAL;
            };

            struct Varyings
            {
                float4 positionCS : SV_POSITION;
                float3 positionWS : TEXCOORD0;
                float3 normalWS   : TEXCOORD1;
            };

            uint hash(uint x)
            {
                x ^= x >> 16;
                x *= 0x45d9f3bu;
                x ^= x >> 16;
                x *= 0x45d9f3bu;
                x ^= x >> 16;
                return x;
            }

            float randomFloat(float2 pixel, int sampleIdx)
            {
                uint h = hash((uint)(pixel.x) * 1973u + (uint)(pixel.y) * 9277u + (uint)(sampleIdx) * 26699u);
                return (float)h / 4294967295.0;
            }

            void buildTangentBasis(float3 N, out float3 T, out float3 B)
            {
                float3 up = abs(N.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
                T = normalize(cross(up, N));
                B = cross(N, T);
            }

            float3 sampleCosineHemisphere(float2 xi, float3 N)
            {
                float phi = 2.0 * PI * xi.x;
                float cosTheta = sqrt(xi.y);
                float sinTheta = sqrt(1.0 - xi.y);

                float3 T, B;
                buildTangentBasis(N, T, B);

                return normalize(T * cos(phi) * sinTheta +
                                 B * sin(phi) * sinTheta +
                                 N * cosTheta);
            }

            float3 sampleGGX(float2 xi, float3 N, float alpha)
            {
                float phi = 2.0 * PI * xi.x;
                float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y));
                float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

                float3 T, B;
                buildTangentBasis(N, T, B);

                return normalize(T * cos(phi) * sinTheta +
                                 B * sin(phi) * sinTheta +
                                 N * cosTheta);
            }

            float D_GGX(float NdotH, float alpha)
            {
                float a2 = alpha * alpha;
                float d = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
                return a2 / (PI * d * d);
            }

            float pdfCosine(float NdotL)
            {
                return NdotL / PI;
            }

            float pdfGGX(float NdotH, float HdotV, float alpha)
            {
                float D = D_GGX(NdotH, alpha);
                return D * NdotH / (4.0 * max(HdotV, 1e-6));
            }

            float3 lookupBrdf(float3 N, float3 V, float3 L)
            {
                float3 H = normalize(V + L);
                float NdotH = max(dot(N, H), 0.0);
                float HdotL = max(dot(H, L), 0.0);

                float theta_h = acos(clamp(NdotH, 0.0, 1.0));
                float theta_d = acos(clamp(HdotL, 0.0, 1.0));

                float3 T;
                if (NdotH > 0.999)
                {
                    float3 up = abs(N.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
                    T = normalize(cross(up, H));
                }
                else
                {
                    T = normalize(N - NdotH * H);
                }
                float3 B = cross(H, T);

                float phi_d = atan2(dot(L, B), dot(L, T));
                if (phi_d < 0.0) phi_d += PI;

                float u = phi_d / PI;
                float v = theta_d / (PI * 0.5);
                float w = sqrt(clamp(theta_h / (PI * 0.5), 0.0, 1.0));

                return SAMPLE_TEXTURE3D(_BrdfLUT, sampler_BrdfLUT, float3(u, v, w)).rgb;
            }

            Varyings vert(Attributes input)
            {
                Varyings output;
                output.positionCS = TransformObjectToHClip(input.positionOS.xyz);
                output.positionWS = TransformObjectToWorld(input.positionOS.xyz);
                output.normalWS = TransformObjectToWorldNormal(input.normalOS);
                return output;
            }

            float4 frag(Varyings input) : SV_Target
            {
                float3 N = normalize(input.normalWS);
                float3 V = normalize(_WorldSpaceCameraPos - input.positionWS);
                float alpha = max(_Roughness * _Roughness, 1e-4);

                float3 Lo = float3(0, 0, 0);
                float2 pixelSeed = input.positionCS.xy;

                for (int i = 0; i < DIFFUSE_SAMPLES; i++)
                {
                    float2 xi;
                    xi.x = randomFloat(pixelSeed + float2(i * 17.3, 0), i);
                    xi.y = randomFloat(pixelSeed + float2(i * 53.1, 0), i + DIFFUSE_SAMPLES + SPECULAR_SAMPLES);

                    float3 L = sampleCosineHemisphere(xi, N);
                    float NdotL = max(dot(N, L), 0.0);
                    if (NdotL <= 0.0) continue;

                    float3 H = normalize(V + L);
                    float NdotH = max(dot(N, H), 0.0);
                    float HdotV = max(dot(H, V), 0.0);

                    float3 brdf = lookupBrdf(N, V, L);
                    float3 envRadiance = SAMPLE_TEXTURECUBE(_EnvCube, sampler_EnvCube, L).rgb;

                    float pCos = pdfCosine(NdotL);
                    float pSpec = pdfGGX(NdotH, HdotV, alpha);

                    float misWeight = (DIFFUSE_SAMPLES * pCos) /
                                      (DIFFUSE_SAMPLES * pCos + SPECULAR_SAMPLES * pSpec);

                    float3 contrib = misWeight * brdf * envRadiance * NdotL / max(pCos * DIFFUSE_SAMPLES, 1e-10);
                    Lo += min(contrib, _MaxSampleValue);
                }

                for (int j = 0; j < SPECULAR_SAMPLES; j++)
                {
                    float2 xi;
                    xi.x = randomFloat(pixelSeed + float2(j * 23.7, 1), j + DIFFUSE_SAMPLES);
                    xi.y = randomFloat(pixelSeed + float2(j * 67.9, 1), j + DIFFUSE_SAMPLES + DIFFUSE_SAMPLES + SPECULAR_SAMPLES);

                    float3 H = sampleGGX(xi, N, alpha);
                    float3 L = reflect(-V, H);
                    float NdotL = max(dot(N, L), 0.0);
                    if (NdotL <= 0.0) continue;

                    float HdotV = max(dot(H, V), 0.0);
                    float NdotH = max(dot(N, H), 0.0);

                    float3 brdf = lookupBrdf(N, V, L);
                    float3 envRadiance = SAMPLE_TEXTURECUBE(_EnvCube, sampler_EnvCube, L).rgb;

                    float pCos = pdfCosine(NdotL);
                    float pSpec = pdfGGX(NdotH, HdotV, alpha);

                    float misWeight = (SPECULAR_SAMPLES * pSpec) /
                                      (DIFFUSE_SAMPLES * pCos + SPECULAR_SAMPLES * pSpec);

                    float3 contrib = misWeight * brdf * envRadiance * NdotL / max(pSpec * SPECULAR_SAMPLES, 1e-10);
                    Lo += min(contrib, _MaxSampleValue);
                }

                // Direct point light
                float3 Ldir = normalize(_LightPos.xyz - input.positionWS);
                float NdotLdir = max(dot(N, Ldir), 0.0);
                if (NdotLdir > 0.0)
                {
                    float3 brdfDirect = lookupBrdf(N, V, Ldir);
                    float dist = length(_LightPos.xyz - input.positionWS);
                    float attenuation = 1.0 / max(dist * dist, 0.001);
                    Lo += brdfDirect * _LightColor.rgb * _LightIntensity * attenuation * NdotLdir;
                }

                float3 color = Lo / (Lo + 1.0);
                color *= _Exposure;

                return float4(color, 1);
            }
            ENDHLSL
        }
    }

    Fallback "Universal Render Pipeline/Lit"
}
