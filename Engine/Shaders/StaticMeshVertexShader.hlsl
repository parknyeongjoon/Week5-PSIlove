// MatrixBuffer: 변환 행렬 관리
cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 M;
    row_major float4x4 VP;
    row_major float4x4 MInverseTranspose;
    bool isSelected;
    float3 MatrixPad0;
};

struct VS_INPUT
{
    float4 position : POSITION; // 버텍스 위치
    float4 color : COLOR; // 버텍스 색상
    float3 normal : NORMAL; // 버텍스 노멀
    float2 texcoord : TEXCOORD;
};

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float4 vertexWorldPosition : VERTEX_POSITION;
    float4 color : COLOR; // 전달할 색상
    float3 normal : NORMAL; // 정규화된 노멀 벡터
    float2 texcoord : TEXCOORD;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    
    // 위치 변환
    output.position = mul(mul(input.position, M),VP);
    output.vertexWorldPosition = mul(input.position, M);
    output.color = input.color;
    if (isSelected)
        output.color *= 0.5;
    
    output.normal = input.normal * 0.5 + 0.5;
    output.texcoord = input.texcoord;
    
    return output;
}