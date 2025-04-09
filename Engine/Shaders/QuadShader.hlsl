//QuadShader.hlsl

Texture2D WorldTexture : register(t1);
SamplerState WorldSampler : register(s0);

struct VS_INPUT
{
    float3 position : POSITION; // 버텍스 위치
    float2 texcoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    //float4 color : COLOR; // 전달할 색상
    //float3 normal : NORMAL; // 정규화된 노멀 벡터
    //float normalFlag : TEXCOORD0; // 노멀 유효성 플래그 (1.0: 유효, 0.0: 무효)
    float2 texcoord : TEXCOORD;
};


PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.position = float4(input.position, 1);
    output.texcoord = float2(input.texcoord);
    
    return output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float4 col = WorldTexture.Sample(WorldSampler, input.texcoord);
       
    return col;
}