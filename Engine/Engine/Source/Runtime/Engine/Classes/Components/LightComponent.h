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
    void SetColor(FLinearColor newColor);
    FLinearColor GetColor() const;
    float GetRadius() const;
    void SetRadius(float r);

private:
    FLinearColor color;
    float radius;
    FBoundingBox AABB;
    
public:
    FBoundingBox GetBoundingBox() const {return AABB;}
    float GetRadius() {return radius;}
    FLinearColor GetColor() {return color;}
};
