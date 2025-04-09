// Auto-generated constant buffer structures with padding
#pragma once

#include "HAL/PlatformType.h"
#include "Math/Matrix.h"
#include "Math/Vector.h"
#include "Math/Vector4.h"

// NOTE: Generated code - do not modify manually.


struct FLighting
{
    FVector Position;
    float  Intensity;
    FVector LightDirection;
    float  AmbientFactor;
    FLinearColor LightColor;
    float  AttenuationRadius;
    FVector pad0;
};

struct alignas(16) FogConstants
{
    float FogDensity; // offset: 0, size: 4
    float FogHeightFalloff; // offset: 4, size: 4
    float StartDistance; // offset: 8, size: 4
    float FogCutoffDistance; // offset: 12, size: 4
    float FogMaxOpacity; // offset: 16, size: 4
    uint8 pad0[12]; // Padding from offset 20 to 32
    FVector4 FogInscatteringColor; // offset: 32, size: 16
    FVector4 CameraWorldPos; // offset: 48, size: 16
    FMatrix InvProjectionMatrix; // offset: 64, size: 64
    FMatrix InvViewMatrix; // offset: 128, size: 64
};

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

struct alignas(16) LightBuffer
{
    FLighting Lights[100]; // offset: 0, size: 6400
    FVector EyePosition; // offset: 6400, size: 12
    float LightCount; // offset: 6412, size: 4
};

struct alignas(16) FUVBuffer
{
    float UOffset; // offset: 0, size: 4
    float VOffset; // offset: 4, size: 4
    float UTiles; // offset: 8, size: 4
    float VTiles; // offset: 12, size: 4
};

struct alignas(16) FMVPConstant
{
    FMatrix M; // offset: 0, size: 64
    FMatrix VP; // offset: 64, size: 64
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

struct alignas(16) FMatrixConstants
{
    FMatrix M; // offset: 0, size: 64
    FMatrix VP; // offset: 64, size: 64
    FMatrix MInverseTranspose; // offset: 128, size: 64
    bool isSelected; // offset: 192, size: 4
    FVector MatrixPad0; // offset: 196, size: 12
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

struct alignas(16) FFlagConstants
{
    bool IsLit; // offset: 0, size: 4
    FVector flagPad0; // offset: 4, size: 12
};

struct alignas(16) FSubMeshConstants
{
    bool IsSelectedSubMesh; // offset: 0, size: 4
    FVector SubMeshPad0; // offset: 4, size: 12
};

enum class EShaderConstantBuffer
{
    FConstants = 0,
    FFlagConstants = 1,
    FGridParametersData = 2,
    FMVPConstant = 3,
    FMaterialConstants = 4,
    FMatrixConstants = 5,
    FPrimitiveCounts = 6,
    FSubMeshConstants = 7,
    FSubUVConstant = 8,
    FUVBuffer = 9,
    FogConstants = 10,
    LightBuffer = 11,
    EShaderConstantBuffer_MAX
};

inline const TCHAR* EShaderConstantBufferToString(EShaderConstantBuffer e)
{
    switch(e)
    {
    case EShaderConstantBuffer::FConstants: return TEXT("FConstants");
    case EShaderConstantBuffer::FFlagConstants: return TEXT("FFlagConstants");
    case EShaderConstantBuffer::FGridParametersData: return TEXT("FGridParametersData");
    case EShaderConstantBuffer::FMVPConstant: return TEXT("FMVPConstant");
    case EShaderConstantBuffer::FMaterialConstants: return TEXT("FMaterialConstants");
    case EShaderConstantBuffer::FMatrixConstants: return TEXT("FMatrixConstants");
    case EShaderConstantBuffer::FPrimitiveCounts: return TEXT("FPrimitiveCounts");
    case EShaderConstantBuffer::FSubMeshConstants: return TEXT("FSubMeshConstants");
    case EShaderConstantBuffer::FSubUVConstant: return TEXT("FSubUVConstant");
    case EShaderConstantBuffer::FUVBuffer: return TEXT("FUVBuffer");
    case EShaderConstantBuffer::FogConstants: return TEXT("FogConstants");
    case EShaderConstantBuffer::LightBuffer: return TEXT("LightBuffer");
    default: return TEXT("unknown");
    }
}

inline EShaderConstantBuffer EShaderConstantBufferFromString(const TCHAR* str)
{
#if USE_WIDECHAR
    if(std::wcscmp(str, TEXT("FConstants")) == 0) return EShaderConstantBuffer::FConstants;
    if(std::wcscmp(str, TEXT("FFlagConstants")) == 0) return EShaderConstantBuffer::FFlagConstants;
    if(std::wcscmp(str, TEXT("FGridParametersData")) == 0) return EShaderConstantBuffer::FGridParametersData;
    if(std::wcscmp(str, TEXT("FMVPConstant")) == 0) return EShaderConstantBuffer::FMVPConstant;
    if(std::wcscmp(str, TEXT("FMaterialConstants")) == 0) return EShaderConstantBuffer::FMaterialConstants;
    if(std::wcscmp(str, TEXT("FMatrixConstants")) == 0) return EShaderConstantBuffer::FMatrixConstants;
    if(std::wcscmp(str, TEXT("FPrimitiveCounts")) == 0) return EShaderConstantBuffer::FPrimitiveCounts;
    if(std::wcscmp(str, TEXT("FSubMeshConstants")) == 0) return EShaderConstantBuffer::FSubMeshConstants;
    if(std::wcscmp(str, TEXT("FSubUVConstant")) == 0) return EShaderConstantBuffer::FSubUVConstant;
    if(std::wcscmp(str, TEXT("FUVBuffer")) == 0) return EShaderConstantBuffer::FUVBuffer;
    if(std::wcscmp(str, TEXT("FogConstants")) == 0) return EShaderConstantBuffer::FogConstants;
    if(std::wcscmp(str, TEXT("LightBuffer")) == 0) return EShaderConstantBuffer::LightBuffer;
#else
    if(std::strcmp(str, "FConstants") == 0) return EShaderConstantBuffer::FConstants;
    if(std::strcmp(str, "FFlagConstants") == 0) return EShaderConstantBuffer::FFlagConstants;
    if(std::strcmp(str, "FGridParametersData") == 0) return EShaderConstantBuffer::FGridParametersData;
    if(std::strcmp(str, "FMVPConstant") == 0) return EShaderConstantBuffer::FMVPConstant;
    if(std::strcmp(str, "FMaterialConstants") == 0) return EShaderConstantBuffer::FMaterialConstants;
    if(std::strcmp(str, "FMatrixConstants") == 0) return EShaderConstantBuffer::FMatrixConstants;
    if(std::strcmp(str, "FPrimitiveCounts") == 0) return EShaderConstantBuffer::FPrimitiveCounts;
    if(std::strcmp(str, "FSubMeshConstants") == 0) return EShaderConstantBuffer::FSubMeshConstants;
    if(std::strcmp(str, "FSubUVConstant") == 0) return EShaderConstantBuffer::FSubUVConstant;
    if(std::strcmp(str, "FUVBuffer") == 0) return EShaderConstantBuffer::FUVBuffer;
    if(std::strcmp(str, "FogConstants") == 0) return EShaderConstantBuffer::FogConstants;
    if(std::strcmp(str, "LightBuffer") == 0) return EShaderConstantBuffer::LightBuffer;
#endif
    return EShaderConstantBuffer::EShaderConstantBuffer_MAX;
}

