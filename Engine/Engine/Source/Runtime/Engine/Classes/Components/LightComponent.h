#pragma once
#include "SceneComponent.h"
#include "Define.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();
    virtual ~ULightComponentBase() override;

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    void InitializeLight();
    void SetColor(FVector4 newColor);
    FVector4 GetColor() const;
    float GetRadius() const;
    void SetRadius(float r);

private:
    FVector4 color;
    float radius;
    FBoundingBox AABB;
    
public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    float GetRadius() {return radius;}
    FVector4 GetColor() {return color;}
};
