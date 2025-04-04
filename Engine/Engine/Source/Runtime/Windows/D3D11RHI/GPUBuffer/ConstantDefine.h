#pragma once
#include "Math/Vector.h"

// 슬롯 번호 가져올 때, CB_GETBINDSLOT(구조체 이름)으로 가져오면 됌
#define CB_GETBINDSLOT(name) __CBUFFERBINDSLOT__##name##__

// 상수 버퍼 용 매크로 -> CBUFFER(구조체 이름, 슬롯 번호)로 상수 버퍼 구조체 선언
#define CBUFFER(name, slot) static const int CB_GETBINDSLOT(name) = slot; struct alignas(16) name

CBUFFER(FSubUVConstant, 1)
{
    float indexU;
    float indexV;
};

struct FGridParameters
{
    float gridSpacing;
    int   numGridLines;
    FVector gridOrigin;
    float pad;
};

struct FPrimitiveCounts 
{
    int BoundingBoxCount;
    int pad;
    int ConeCount; 
    int pad1;
};

struct FMaterialConstants
{
    FVector DiffuseColor;
    float TransparencyScalar;
    FVector AmbientColor;
    float DensityScalar;
    FVector SpecularColor;
    float SpecularScalar;
    FVector EmmisiveColor;
    float MaterialPad0;
};

struct FCone
{
    FVector ConeApex; // 원뿔의 꼭짓점
    float ConeRadius; // 원뿔 밑면 반지름

    FVector ConeBaseCenter; // 원뿔 밑면 중심
    float ConeHeight; // 원뿔 높이 (Apex와 BaseCenter 간 차이)
    FVector4 Color;

    int ConeSegmentCount; // 원뿔 밑면 분할 수
    float pad[3];

};

struct FLighting
{
    float lightDirX, lightDirY, lightDirZ; // 조명 방향
    float pad1;                      // 16바이트 정렬용 패딩
    float lightColorX, lightColorY, lightColorZ;    // 조명 색상
    float pad2;                      // 16바이트 정렬용 패딩
    float AmbientFactor;             // ambient 계수
    float pad3; // 16바이트 정렬 맞춤 추가 패딩
    float pad4; // 16바이트 정렬 맞춤 추가 패딩
    float pad5; // 16바이트 정렬 맞춤 추가 패딩
};

struct FConstants
{
    FMatrix MVP;      // 모델
    FMatrix ModelMatrixInverseTranspose; // normal 변환을 위한 행렬
    FVector4 UUIDColor;
    bool IsSelected;
    FVector pad;
};

struct FLitUnlitConstants
{
    int isLit; // 1 = Lit, 0 = Unlit 
    FVector pad;
};

struct FSubMeshConstants
{
    float isSelectedSubMesh;
    FVector pad;
};

struct FTextureConstants
{
    float UOffset;
    float VOffset;
    float pad0;
    float pad1;
};
