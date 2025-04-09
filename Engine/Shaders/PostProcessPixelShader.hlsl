#include "ShaderHeaders/Samplers.hlsli"
#include "PostProcessVertexShader.hlsl"

Texture2D SceneColorTexture : register(t0);

float4 mainPS(PS_INPUT input) : SV_Target
{
    float4 sceneColor = SceneColorTexture.Sample(linearSampler, input.UV);
    return sceneColor;
}