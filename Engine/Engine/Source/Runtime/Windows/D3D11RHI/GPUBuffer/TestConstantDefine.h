// Auto-generated constant buffer structures with padding
#pragma once

#include "HAL/PlatformType.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Math/Vector4.h"

// NOTE: Generated code - do not modify manually.

struct alignas(16) FSubUVConstant
{
    float indexU; // offset: 0, size: 4
    float indexV; // offset: 4, size: 4
    uint8 pad0[8]; // Padding to end of buffer
};

struct alignas(16) FConstants
{
    FMatrix MVP; // offset: 0, size: 64
    float Flag; // offset: 64, size: 4
    uint8 pad0[12]; // Padding to end of buffer
};

struct alignas(16) FMVPConstant
{
    FMatrix MVP; // offset: 0, size: 64
};

struct alignas(16) FGridParametersData
{
    float GridSpacing; // offset: 0, size: 4
    int GridCount; // offset: 4, size: 4
    uint8 pad0[8]; // Padding from offset 8 to 16
    FVector GridOrigin; // offset: 16, size: 12
    float Padding; // offset: 28, size: 4
};

struct alignas(16) FPrimitiveCounts
{
    int BoundingBoxCount; // offset: 0, size: 4
    int pad; // offset: 4, size: 4
    int ConeCount; // offset: 8, size: 4
    int pad1; // offset: 12, size: 4
};

struct alignas(16) FMatrixBuffer
{
    FMatrix MVP; // offset: 0, size: 64
    FMatrix MInverseTranspose; // offset: 64, size: 64
    FVector4 UUID; // offset: 128, size: 16
    bool isSelected; // offset: 144, size: 4
    FVector MatrixPad0; // offset: 148, size: 12
};

struct alignas(16) FLightingBuffer
{
    FVector LightDirection; // offset: 0, size: 12
    float AmbientFactor; // offset: 12, size: 4
    FVector LightColor; // offset: 16, size: 12
    float LightPad0; // offset: 28, size: 4
};

struct alignas(16) FFlagConstants
{
    bool IsLit; // offset: 0, size: 4
    FVector FlagPad0; // offset: 4, size: 12
};

struct alignas(16) FMatrixConstants
{
    FMatrix MVP; // offset: 0, size: 64
    FMatrix MInverseTranspose; // offset: 64, size: 64
    FVector4 UUID; // offset: 128, size: 16
    bool isSelected; // offset: 144, size: 4
    FVector MatrixPad0; // offset: 148, size: 12
};

struct alignas(16) FMaterialConstants
{
    FVector DiffuseColor; // offset: 0, size: 12
    float TransparencyScalar; // offset: 12, size: 4
    FVector AmbientColor; // offset: 16, size: 12
    float DensityScalar; // offset: 28, size: 4
    FVector SpecularColor; // offset: 32, size: 12
    float SpecularScalar; // offset: 44, size: 4
    FVector EmissiveColor; // offset: 48, size: 12
    float MaterialPad0; // offset: 60, size: 4
};

struct alignas(16) FSubMeshConstants
{
    bool IsSelectedSubMesh; // offset: 0, size: 4
    FVector SubMeshPad0; // offset: 4, size: 12
};

struct alignas(16) FTextureConstants
{
    FVector2D UVOffset; // offset: 0, size: 8
    FVector2D TexturePad0; // offset: 8, size: 8
};

enum class EShaderConstantBuffer
{
    FConstants = 0,
    FFlagConstants = 1,
    FGridParametersData = 2,
    FLightingBuffer = 3,
    FMVPConstant = 4,
    FMaterialConstants = 5,
    FMatrixBuffer = 6,
    FMatrixConstants = 7,
    FPrimitiveCounts = 8,
    FSubMeshConstants = 9,
    FSubUVConstant = 10,
    FTextureConstants = 11,
    EShaderConstantBuffer_MAX
};

inline const TCHAR* EShaderConstantBufferToString(EShaderConstantBuffer e)
{
    switch(e)
    {
    case EShaderConstantBuffer::FConstants: return TEXT("FConstants");
    case EShaderConstantBuffer::FFlagConstants: return TEXT("FFlagConstants");
    case EShaderConstantBuffer::FGridParametersData: return TEXT("FGridParametersData");
    case EShaderConstantBuffer::FLightingBuffer: return TEXT("FLightingBuffer");
    case EShaderConstantBuffer::FMVPConstant: return TEXT("FMVPConstant");
    case EShaderConstantBuffer::FMaterialConstants: return TEXT("FMaterialConstants");
    case EShaderConstantBuffer::FMatrixBuffer: return TEXT("FMatrixBuffer");
    case EShaderConstantBuffer::FMatrixConstants: return TEXT("FMatrixConstants");
    case EShaderConstantBuffer::FPrimitiveCounts: return TEXT("FPrimitiveCounts");
    case EShaderConstantBuffer::FSubMeshConstants: return TEXT("FSubMeshConstants");
    case EShaderConstantBuffer::FSubUVConstant: return TEXT("FSubUVConstant");
    case EShaderConstantBuffer::FTextureConstants: return TEXT("FTextureConstants");
    default: return TEXT("unknown");
    }
}

inline EShaderConstantBuffer EShaderConstantBufferFromString(const TCHAR* str)
{
#if USE_WIDECHAR
    if(std::wcscmp(str, TEXT("FConstants")) == 0) return EShaderConstantBuffer::FConstants;
    if(std::wcscmp(str, TEXT("FFlagConstants")) == 0) return EShaderConstantBuffer::FFlagConstants;
    if(std::wcscmp(str, TEXT("FGridParametersData")) == 0) return EShaderConstantBuffer::FGridParametersData;
    if(std::wcscmp(str, TEXT("FLightingBuffer")) == 0) return EShaderConstantBuffer::FLightingBuffer;
    if(std::wcscmp(str, TEXT("FMVPConstant")) == 0) return EShaderConstantBuffer::FMVPConstant;
    if(std::wcscmp(str, TEXT("FMaterialConstants")) == 0) return EShaderConstantBuffer::FMaterialConstants;
    if(std::wcscmp(str, TEXT("FMatrixBuffer")) == 0) return EShaderConstantBuffer::FMatrixBuffer;
    if(std::wcscmp(str, TEXT("FMatrixConstants")) == 0) return EShaderConstantBuffer::FMatrixConstants;
    if(std::wcscmp(str, TEXT("FPrimitiveCounts")) == 0) return EShaderConstantBuffer::FPrimitiveCounts;
    if(std::wcscmp(str, TEXT("FSubMeshConstants")) == 0) return EShaderConstantBuffer::FSubMeshConstants;
    if(std::wcscmp(str, TEXT("FSubUVConstant")) == 0) return EShaderConstantBuffer::FSubUVConstant;
    if(std::wcscmp(str, TEXT("FTextureConstants")) == 0) return EShaderConstantBuffer::FTextureConstants;
#else
    if(std::strcmp(str, "FConstants") == 0) return EShaderConstantBuffer::FConstants;
    if(std::strcmp(str, "FFlagConstants") == 0) return EShaderConstantBuffer::FFlagConstants;
    if(std::strcmp(str, "FGridParametersData") == 0) return EShaderConstantBuffer::FGridParametersData;
    if(std::strcmp(str, "FLightingBuffer") == 0) return EShaderConstantBuffer::FLightingBuffer;
    if(std::strcmp(str, "FMVPConstant") == 0) return EShaderConstantBuffer::FMVPConstant;
    if(std::strcmp(str, "FMaterialConstants") == 0) return EShaderConstantBuffer::FMaterialConstants;
    if(std::strcmp(str, "FMatrixBuffer") == 0) return EShaderConstantBuffer::FMatrixBuffer;
    if(std::strcmp(str, "FMatrixConstants") == 0) return EShaderConstantBuffer::FMatrixConstants;
    if(std::strcmp(str, "FPrimitiveCounts") == 0) return EShaderConstantBuffer::FPrimitiveCounts;
    if(std::strcmp(str, "FSubMeshConstants") == 0) return EShaderConstantBuffer::FSubMeshConstants;
    if(std::strcmp(str, "FSubUVConstant") == 0) return EShaderConstantBuffer::FSubUVConstant;
    if(std::strcmp(str, "FTextureConstants") == 0) return EShaderConstantBuffer::FTextureConstants;
#endif
    return EShaderConstantBuffer::EShaderConstantBuffer_MAX;
}

