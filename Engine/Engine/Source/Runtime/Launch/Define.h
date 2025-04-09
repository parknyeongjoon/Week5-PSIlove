#pragma once
#include <cmath>
#include <algorithm>
#include "Core/Container/String.h"
#include "Core/Container/Array.h"
#include "UObject/NameTypes.h"

// 수학 관련
#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"


#define UE_LOG Console::GetInstance().AddLog

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "UserInterface/Console.h"

class UWorld;

enum class EWorldType
{
    Editor,
    EditorPreview,
    PIE,
    Game,
};

enum class GBufferType : uint8
{
    None,
    Position,
    Diffuse,
    Normal,

    Max,
};

struct FVertexSimple
{
    float x, y, z;    // Position
    float r, g, b, a; // Color
    float nx, ny, nz;
    float u=0, v=0;
    uint32 MaterialIndex;
};

struct FLinearColor;
struct FColor
{
    uint8 r;
    uint8 g;
    uint8 b;
    uint8 a;

    FLinearColor ConvertToFLinearColor() const;
};

struct FLinearColor
{
    float r;
    float g;
    float b;
    float a;

    FLinearColor() : r(0), g(0), b(0), a(1) {}
    FLinearColor(float InR, float InG, float InB, float InA = 1.0f)
        : r(InR), g(InG), b(InB), a(InA) {
    }

    FColor ConvertToFColor() const;
};

// Material Subset
struct FMaterialSubset
{
    uint32 IndexStart; // Index Buffer Start pos
    uint32 IndexCount; // Index Count
    uint32 MaterialIndex; // Material Index
    FString MaterialName; // Material Name
};

struct FStaticMaterial
{
    class UMaterial* Material;
    FName MaterialSlotName;
    //FMeshUVChannelInfo UVChannelData;
};

// OBJ File Raw Data
struct FObjInfo
{
    FWString ObjectName; // OBJ File Name
    FWString PathName; // OBJ File Paths
    FString DisplayName; // Display Name
    FString MatName; // OBJ MTL File Name
    
    // Group
    uint32 NumOfGroup = 0; // token 'g' or 'o'
    TArray<FString> GroupName;
    
    // Vertex, UV, Normal List
    TArray<FVector> Vertices;
    TArray<FVector> Normals;
    TArray<FVector2D> UVs;
    
    // Faces
    TArray<int32> Faces;

    // Index
    TArray<uint32> VertexIndices;
    TArray<uint32> NormalIndices;
    TArray<uint32> TextureIndices;
    
    // Material
    TArray<FMaterialSubset> MaterialSubsets;
};

struct FObjMaterialInfo
{
    FString MTLName;  // newmtl : Material Name.

    bool bHasTexture = false;  // Has Texture?
    bool bTransparent = false; // Has alpha channel?

    FVector Diffuse;  // Kd : Diffuse (Vector4)
    FVector Specular;  // Ks : Specular (Vector) 
    FVector Ambient;   // Ka : Ambient (Vector)
    FVector Emissive;  // Ke : Emissive (Vector)

    float SpecularScalar; // Ns : Specular Power (Float)
    float DensityScalar;  // Ni : Optical Density (Float)
    float TransparencyScalar; // d or Tr  : Transparency of surface (Float)

    uint32 IlluminanceModel; // illum: illumination Model between 0 and 10. (UINT)

    /* Texture */
    FString DiffuseTextureName;  // map_Kd : Diffuse texture
    FWString DiffuseTexturePath;
    
    FString AmbientTextureName;  // map_Ka : Ambient texture
    FWString AmbientTexturePath;
    
    FString SpecularTextureName; // map_Ks : Specular texture
    FWString SpecularTexturePath;
    
    FString BumpTextureName;     // map_Bump : Bump texture
    FWString BumpTexturePath;
    
    FString AlphaTextureName;    // map_d : Alpha texture
    FWString AlphaTexturePath;
};

struct FWorldContext
{
    UWorld* World;
    EWorldType worldType;
    
};

enum class EShaderStage
{
    VS,		// Vertex Shader
    HS,		// Hull Shader
    DS,		// Domain Shader
    GS,		// Geometry Shader
    PS,		// Pixel Shader
    CS,		// Compute Shader
    All,
    End,
};

// ShaderType과 Constant 이름을 결합한 키 구조체
struct FShaderConstantKey
{
    EShaderStage ShaderType;  // 예: Vertex, Pixel 등
    FString ConstantName;    // 상수 버퍼 내 상수 이름

    // 동등 비교 연산자: 두 키가 동일하면 true
    bool operator==(const FShaderConstantKey& Other) const
    {
        return ShaderType == Other.ShaderType && ConstantName == Other.ConstantName;
    }
};

// std::hash 특수화를 통해 FShaderConstantKey를 해시 기반 컨테이너에서 사용할 수 있게 함
namespace std
{
    template<>
    struct hash<FShaderConstantKey>
    {
        std::size_t operator()(const FShaderConstantKey& Key) const noexcept
        {
            // EShaderType은 enum class이므로 int로 캐스팅하여 해시를 계산
            std::size_t h1 = std::hash<int>()(static_cast<int>(Key.ShaderType));
            std::size_t h2 = std::hash<FString>()(Key.ConstantName);
            // 간단한 해시 결합: XOR과 쉬프트 사용 (더 복잡한 해시 결합도 가능)
            return h1 ^ (h2 << 1);
        }
    };
}

enum class ESamplerType
{
    Point,
    Linear,
    Anisotropic,
    PostProcess,
    End,
};

enum class ERenderingMode
{
    Opaque,
    CutOut,
    Transparent,
    PostProcess,
    End,
};

enum class ETextureType
{
    Albedo,
    Normal,
    Specular,
    Smoothness,
    Metallic,
    Sprite,
    End,
};

enum class ERasterizerState
{
    SolidBack,
    SolidFront,
    SolidNone,
    WireFrame,
    End,
};

enum class EBlendState
{
    AlphaBlend,
    OneOne,
    End,
};

enum class EDepthStencilState
{
    DepthNone,
    LessEqual,
    End,
};

// Cooked Data
namespace OBJ
{
    struct FStaticMeshRenderData
    {
        FWString ObjectName;
        FWString PathName;
        FString DisplayName;
        
        TArray<FVertexSimple> Vertices;
        TArray<uint32> Indices;

        // Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer;
        // Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer;
        
        TArray<FObjMaterialInfo> Materials;
        TArray<FMaterialSubset> MaterialSubsets;

        FVector BoundingBoxMin;
        FVector BoundingBoxMax;
    };
}

struct FVertexTexture
{
	float x, y, z;    // Position
	float u, v; // Texture
};

struct FScreenVertex
{
    FVector4 Position;
    float TexcoordU;
    float TexcoordV;
};

struct FSimpleVertex
{
    float dummy; // 내용은 사용되지 않음
    float padding[11];
};

struct FRect
{
    FRect() : leftTopX(0), leftTopY(0), width(0), height(0) {}
    FRect(float x, float y, float w, float h) : leftTopX(x), leftTopY(y), width(w), height(h) {}
    float leftTopX, leftTopY, width, height;
};
struct FPoint
{
    FPoint() : x(0), y(0) {}
    FPoint(float _x, float _y) : x(_x), y(_y) {}
    FPoint(long _x, long _y) : x(_x), y(_y) {}
    FPoint(int _x, int _y) : x(_x), y(_y) {}

    float x, y;
};
struct FBoundingBox
{
    FBoundingBox(){}
    FBoundingBox(FVector _min, FVector _max) : min(_min), max(_max) {}
	FVector min; // Minimum extents
	float pad;
	FVector max; // Maximum extents
	float pad1;
    bool Intersect(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const;
};

struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    FLinearColor Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];

};

struct FOBB
{
    FVector corners[8];
};