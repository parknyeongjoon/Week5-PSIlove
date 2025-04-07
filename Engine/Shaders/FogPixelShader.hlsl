#include "PostProcessVertexShader.hlsl"

struct FogParams
{
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;
    float4 FogInscatteringColor;
};

cbuffer FogConstants : register(b0)
{
    FogParams Fog;
    float4 CameraWorldPos;
    row_major float4x4 InvProjectionMatrix;
    row_major float4x4 InvViewMatrix;

};

Texture2D SceneColorTexture : register(t0);
Texture2D SceneDepthTexture : register(t1);
SamplerState SceneSampler : register(s0);

float3 ReconstructViewPos(float2 uv, float depth)
{
    float4 ndc = float4(uv * 2.0f - 1.0f, depth, 1.0f);
    float4 viewPos = mul(ndc, InvProjectionMatrix);
    viewPos /= viewPos.w;
    float4 worldPos = mul(viewPos, InvViewMatrix);
    worldPos /= worldPos.w;

    return worldPos.xyz;
}

float4 MainPS(PS_INPUT input) : SV_Target
{
    float sceneDepth = SceneDepthTexture.Sample(SceneSampler, input.UV).r;
    float3 worldPos = ReconstructViewPos(input.UV, sceneDepth);

    // Distance from camera
    float distance = length(worldPos - CameraWorldPos);
    if (distance < Fog.StartDistance || distance > Fog.FogCutoffDistance)
        return SceneColorTexture.Sample(SceneSampler, input.UV);

    // Height falloff
    float heightDiff = max(0.0, worldPos.z);
    float heightFog = exp(-heightDiff * Fog.FogHeightFalloff);
    float distanceFog = exp(-distance * Fog.FogDensity);
    float fogFactor = saturate(1.0 - distanceFog * heightFog);

    // Apply max opacity clamp
    fogFactor = min(fogFactor, Fog.FogMaxOpacity);

    // Apply fog color
    float4 sceneColor = SceneColorTexture.Sample(SceneSampler, input.UV);
    float4 fogColor = Fog.FogInscatteringColor;

    float4 finalColor = lerp(sceneColor, fogColor, fogFactor);
    //return float4(1, 0, 0, 1);
    return finalColor;
    // return sceneColor;
}