#pragma once
#include "PrimitiveComponent.h"
#include "Define.h"

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void InitializeLight();
    void SetColor(FColor newColor);
    FColor GetColor() const;
    float GetRadius() const;
    void SetRadius(float r);

private:
    FColor color;
    float radius;
    FBoundingBox AABB;
    
public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    float GetRadius() {return radius;}
    FColor GetColor() {return color;}
};
