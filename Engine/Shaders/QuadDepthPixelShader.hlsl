#include "ShaderHeaders/Samplers.hlsli"

Texture2D<float> SceneDepthTexture : register(t0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

float4 mainPS(PS_INPUT input) : SV_Target
{
    float depth = SceneDepthTexture.Sample(linearSampler, input.texcoord);
    
    depth = pow(depth, 0.25); // 밝기 조정 (선택사항)
    depth = saturate(depth); // 0~1 범위로 제한

    depth = (1 - depth) * 100;
    //return float4(1 - depth, 1 - depth, 1 - depth, 1); // depth 값을 R 채널에만 출력 
    return float4(depth, depth, depth, 1.0);
    //if (depth < 0.3f)
    //    return float4(1, 0, 0, 1); // 가까우면 빨간색
    //else if (depth < 0.6f)
    //    return float4(0, 1, 0, 1); // 중간이면 초록색
    //else
    //    return float4(0, 0, 1, 1); // 멀면 파란색

    //return float4(depth, depth, depth, 1.0f); 
}
