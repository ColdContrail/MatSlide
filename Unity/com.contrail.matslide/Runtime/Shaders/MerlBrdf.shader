Shader "Custom/MerlBrdf"
{
    Properties
    {
        _BrdfLUT ("BRDF LUT", 3D) = "white" {}
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
            #include "Packages/com.unity.render-pipelines.universal/ShaderLibrary/Lighting.hlsl"

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

            TEXTURE3D(_BrdfLUT);
            SAMPLER(sampler_BrdfLUT);

            float4 _LightPos;
            float4 _LightColor;
            float _LightIntensity;

            #define PI 3.14159265359

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
                float3 L = normalize(_LightPos.xyz - input.positionWS);
                float3 H = normalize(V + L);

                float NdotL = max(dot(N, L), 0.0);
                if (NdotL <= 0.0)
                    return float4(0, 0, 0, 1);

                float theta_h = acos(clamp(dot(N, H), 0.0, 1.0));
                float theta_d = acos(clamp(dot(H, L), 0.0, 1.0));

                // Build local frame around H
                float3 T;
                if (dot(N, H) > 0.999)
                {
                    float3 up = abs(N.y) < 0.999 ? float3(0, 1, 0) : float3(1, 0, 0);
                    T = normalize(cross(up, H));
                }
                else
                {
                    T = normalize(N - dot(N, H) * H);
                }
                float3 B_ = cross(H, T);

                float phi_d = atan2(dot(L, B_), dot(L, T));
                if (phi_d < 0.0)
                    phi_d += PI;

                float u = phi_d / PI;
                float v = theta_d / (PI * 0.5);
                float w = sqrt(clamp(theta_h / (PI * 0.5), 0.0, 1.0));

                float3 brdf = SAMPLE_TEXTURE3D(_BrdfLUT, sampler_BrdfLUT, float3(u, v, w)).rgb;

                float distance = length(_LightPos.xyz - input.positionWS);
                float attenuation = 1.0 / max(distance * distance, 0.001);
                float3 irradiance = _LightColor.rgb * _LightIntensity * attenuation;

                float3 Lo = brdf * irradiance * NdotL;

                // Reinhard tone mapping (gamma handled by Unity linear pipeline)
                float3 color = Lo / (Lo + float3(1, 1, 1));

                return float4(color, 1);
            }
            ENDHLSL
        }
    }

    Fallback "Universal Render Pipeline/Lit"
}
