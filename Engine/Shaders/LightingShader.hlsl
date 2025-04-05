Texture2D PositionBuffer : register(t0);
Texture2D NormalBuffer : register(t1);
Texture2D DiffuseBuffer : register(t2);
Texture2D MaterialBuffer : register(t3);
SamplerState SamLinear : register(s0);

struct PS_INPUT
{
    float2 texCoord : TEXCOORD0;
};

struct PS_OUTPUT
{
    float4 color : SV_TARGET0;
};

PS_OUTPUT main(PS_INPUT input)
{
    PS_OUTPUT output;

    output.color = float4(1, 1, 1, 1);
    return output;
}