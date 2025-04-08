#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UMaterial : public UObject {
    DECLARE_CLASS(UMaterial, UObject)

public:
    UMaterial() {}
    ~UMaterial() {}

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

public:
    FObjMaterialInfo& GetMaterialInfo() { return materialInfo; }
    void SetMaterialInfo(const FObjMaterialInfo& InValue) { materialInfo = InValue; }

    // 색상 및 재질 속성 설정자
    void SetDiffuse(const FVector& DiffuseIn) { materialInfo.Diffuse = DiffuseIn; }
    void SetSpecular(const FVector& SpecularIn) { materialInfo.Specular = SpecularIn; }
    void SetAmbient(const FVector& AmbientIn) { materialInfo.Ambient = AmbientIn; }
    void SetEmissive(const FVector& EmissiveIn) { materialInfo.Emissive = EmissiveIn; }

    // 스칼라 속성 설정자
    void SetSpecularPower(const float InSpecularPower) { materialInfo.SpecularScalar = InSpecularPower; }
    void SetOpticalDensity(const float InDensity) { materialInfo.DensityScalar = InDensity; }
    void SetTransparency(const float InTransparency)
    {
        materialInfo.TransparencyScalar = InTransparency;
        materialInfo.bTransparent = (InTransparency < 1.0f);
    }

    FString GetMaterialName() { return materialInfo.MTLName; }
    void SetMaterialName(const FString& InMaterialName) { materialInfo.MTLName = InMaterialName; }
    
    bool IsHasTexture() const { return materialInfo.bHasTexture; }
    bool IsTransparent() const { return materialInfo.bTransparent; }

    FVector GetDiffuse() const { return materialInfo.Diffuse; }
    FVector GetSpecular() const { return materialInfo.Specular; }
    FVector GetAmbient() const { return materialInfo.Ambient; }
    FVector GetEmissive() const { return materialInfo.Emissive; }

    float GetSpecularScalar() const { return materialInfo.SpecularScalar; }
    float GetDensity() const { return materialInfo.DensityScalar; }
    float GetTransparency() const { return materialInfo.bTransparent; }

    uint32 GetIlluminanceModel() const { return materialInfo.IlluminanceModel; }

    FString GetDiffuseTextureName() const { return materialInfo.DiffuseTextureName; }
    FString GetSpecularTextureName() const { return materialInfo.SpecularTextureName; }
    FString GetAmbientTextureName() const { return materialInfo.AmbientTextureName; }
    FString GetBumpTextureName() const { return materialInfo.BumpTextureName; }
    FString GetAlphaTextureName() const { return materialInfo.AlphaTextureName; }
private:
    FObjMaterialInfo materialInfo;

};