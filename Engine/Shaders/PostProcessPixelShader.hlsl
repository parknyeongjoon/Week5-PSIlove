#include "PostProcessVertexShader.hlsl"

Texture2D SceneColorTexture : register(t0);
SamplerState SceneSampler : register(s0);

float4 MainPS(PS_INPUT input) : SV_Target
{
    float4 sceneColor = SceneColorTexture.Sample(SceneSampler, input.UV);
    return sceneColor;
}