#pragma once
#include "SceneComponent.h"
#include "Define.h"

class ULightComponent : public USceneComponent
{
    DECLARE_CLASS(ULightComponent, USceneComponent)
    
public:
    ULightComponent();

    virtual UObject* Duplicate() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void InitializeLight();

private:
    FLinearColor LightColor;
    float Intensity = 3.0f;
    float AttenuationRadius;
    FBoundingBox AABB;
    
public:
    UPROPERTY(FLinearColor, LightColor);
    UPROPERTY(float, Intensity);
    UPROPERTY(float, AttenuationRadius);
    FBoundingBox GetBoundingBox() const {return AABB;}
};
