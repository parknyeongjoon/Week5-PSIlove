struct VS_INPUT
{
    float4 Pos : POSITION;
    float2 UV : TEXCOORD0;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float2 UV : TEXCOORD0;
};

PS_INPUT mainVS(VS_INPUT input)
{
    PS_INPUT output;
    output.Pos = input.Pos;
    output.UV = input.UV;
    return output;
}