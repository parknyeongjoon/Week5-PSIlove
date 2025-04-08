#include "ShaderHeaders/ConstantBuffers.hlsli"
#include "ShaderHeaders/Samplers.hlsli"

Texture2D gTexture : register(t0);

cbuffer FSubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}
struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

float4 mainPS(PSInput input) : SV_TARGET
{
    float2 uv = input.texCoord + float2(indexU, indexV);
    float4 col = gTexture.Sample(linearSampler, uv);
    float threshold = 0.1; // 필요한 경우 임계값을 조정
    if (col.a < threshold)
        clip(-1); // 픽셀 버리기
    
    return col;
}
