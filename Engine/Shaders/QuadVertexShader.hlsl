struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer FUVBuffer : register(b0)
{
    float UOffset;
    float VOffset;
    float UTiles;
    float VTiles;
}

PSInput mainVS(uint vertexID : SV_VertexID)
{
    PSInput output;
    
    // 삼각형 스트립을 이용한 풀스크린 Quad (NDC 공간에서 직접 생성)
    float2 positions[6] =
    {
        float2(-1, 1), // Top Left
        float2(1, 1), // Top Right
        float2(-1, -1), // Bottom Left
        float2(1, 1), // Top Right
        float2(1, -1), // Bottom Right
        float2(-1, -1) // Bottom Left
    };
    
    float2 uvs[6] =
    {
        float2(0, 0), float2(1, 0), float2(0, 1),
        float2(1, 0), float2(1, 1), float2(0, 1)
    };
    
    output.position = float4(positions[vertexID], 0, 1);
    output.texCoord = float2(UOffset + uvs[vertexID].x * UTiles, VOffset + uvs[vertexID].y * VTiles);
    
    return output;
}