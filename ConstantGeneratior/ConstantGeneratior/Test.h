// Auto-generated constant buffer structures with padding
#pragma once

// NOTE: Generated code - do not modify manually.

struct alignas(16) FSubUVConstant
{
    float indexU; // offset: 0, size: 4
    float indexV; // offset: 4, size: 4
    uint8_t pad0[8]; // Padding to end of buffer
};

struct alignas(16) FConstants
{
    FMatrix MVP; // offset: 0, size: 64
    float Flag; // offset: 64, size: 4
    uint8_t pad0[12]; // Padding to end of buffer
};

struct alignas(16) FMatrixBuffer
{
    FMatrix MVP; // offset: 0, size: 64
};

struct alignas(16) FGridParametersData
{
    float GridSpacing; // offset: 0, size: 4
    int GridCount; // offset: 4, size: 4
    uint8_t pad0[8]; // Padding from offset 8 to 16
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

struct alignas(16) g_BoundingBoxes
{
    UNKNOWN $Element; // offset: 0, size: 32
};

struct alignas(16) g_ConeData
{
    UNKNOWN $Element; // offset: 0, size: 64
};

struct alignas(16) g_OrientedBoxes
{
    UNKNOWN $Element; // offset: 0, size: 96
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
    UNKNOWN Material; // offset: 0, size: 64
};

struct alignas(16) FLightingConstants
{
    FVector LightDirection; // offset: 0, size: 12
    float LightPad0; // offset: 12, size: 4
    FVector LightColor; // offset: 16, size: 12
    float LightPad1; // offset: 28, size: 4
    float AmbientFactor; // offset: 32, size: 4
    FVector LightPad2; // offset: 36, size: 12
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

