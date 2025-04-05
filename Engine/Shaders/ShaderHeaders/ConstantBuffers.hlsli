//Font
cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
    float Flag;
}

cbuffer SubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}

cbuffer UUIDConstant : register(b2)
{
    float4 UUID;
}

//Line
cbuffer MatrixBuffer : register(b0)
{
    row_major float4x4 MVP;
};

cbuffer GridParametersData : register(b1)
{
    float GridSpacing;
    int GridCount; // 총 grid 라인 수
    float3 GridOrigin; // Grid의 중심
    float Padding;
};
cbuffer PrimitiveCounts : register(b3)
{
    int BoundingBoxCount; // 렌더링할 AABB의 개수
    int pad;
    int ConeCount; // 렌더링할 cone의 개수
    int pad1;
};

//ShaderW0
// MatrixBuffer: 변환 행렬 관리
cbuffer MatrixBuffer : register(b0)
{
    row_major float4x4 MVP;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

// LightingBuffer: 조명 관련 파라미터 관리
cbuffer LightingBuffer : register(b1)
{
    float3 LightDirection; // 조명 방향 (단위 벡터; 빛이 들어오는 방향의 반대 사용)
    float AmbientFactor; // ambient 계수 (예: 0.1)
    float3 LightColor; // 조명 색상 (예: (1, 1, 1))
    float LightPad0; // 16바이트 정렬용 패딩
};

cbuffer FFlagConstants : register(b2)
{
    bool IsLit;
    float3 FlagPad0;
}

//StaticMesh
cbuffer MatrixConstants : register(b0)
{
    row_major float4x4 MVP;
    row_major float4x4 MInverseTranspose;
    float4 UUID;
    bool isSelected;
    float3 MatrixPad0;
};

struct FMaterial
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 AmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    float MaterialPad0;
};

cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer LightingConstants : register(b2)
{
    float3 LightDirection; // 조명 방향 (단위 벡터; 빛이 들어오는 방향의 반대 사용)
    float LightPad0; // 16바이트 정렬용 패딩
    float3 LightColor; // 조명 색상 (예: (1, 1, 1))
    float LightPad1; // 16바이트 정렬용 패딩
    float AmbientFactor; // ambient 계수 (예: 0.1)
    float3 LightPad2; // 16바이트 정렬 맞춤 추가 패딩
};

cbuffer FlagConstants : register(b3)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b4)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b5)
{
    float2 UVOffset;
    float2 TexturePad0;
}

//Texture
cbuffer constants : register(b0)
{
    row_major float4x4 MVP;
    float Flag;
}

cbuffer SubUVConstant : register(b1)
{
    float indexU;
    float indexV;
}

cbuffer UUIDConstant : register(b2)
{
    float4 UUID;
}