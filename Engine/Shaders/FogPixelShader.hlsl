#include "ShaderHeaders/Samplers.hlsli"
#include "PostProcessVertexShader.hlsl"

Texture2D SceneColorTexture : register(t0);
Texture2D SceneDepthTexture : register(t1);

cbuffer FFogConstants : register(b0)
{
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float4 FogInscatteringColor;
    float4 CameraWorldPos;
    row_major float4x4 InvProjectionMatrix;
    row_major float4x4 InvViewMatrix;
};

float3 ReconstructViewPos(float2 uv, float depth)
{
    float4 ndc = float4(uv * 2.0f - 1.0f, depth, 1.0f);
    float4 viewPos = mul(ndc, InvProjectionMatrix);
    viewPos /= viewPos.w;
    float4 worldPos = mul(viewPos, InvViewMatrix);
    worldPos /= worldPos.w;

    return worldPos.xyz;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float sceneDepth = SceneDepthTexture.Sample(linearSampler, input.UV).r;
    float3 worldPos = ReconstructViewPos(input.UV, sceneDepth);

    // Distance from camera
    float distance = length(worldPos - CameraWorldPos);
    if (distance < StartDistance || distance > FogCutoffDistance)
        return SceneColorTexture.Sample(linearSampler, input.UV);

    // Height falloff
    float heightDiff = max(0.0, worldPos.z);
    float heightFog = exp(-heightDiff * FogHeightFalloff);
    float distanceFog = exp(-distance * FogDensity);
    float fogFactor = saturate(1.0 - distanceFog * heightFog);

    // Apply max opacity clamp
    fogFactor = min(fogFactor, FogMaxOpacity);

    // Apply fog color
    float4 sceneColor = SceneColorTexture.Sample(linearSampler, input.UV);
    float4 fogColor = FogInscatteringColor;

    float4 finalColor = lerp(sceneColor, fogColor, fogFactor);
    //return float4(1, 0, 0, 1);
    return finalColor;
    // return sceneColor;
}