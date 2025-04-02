#pragma once
#include "TextComponent.h"

class UTextUUIDComponent : public UTextComponent
{
    DECLARE_CLASS(UTextUUIDComponent, UTextComponent)

public:
    UTextUUIDComponent();
    virtual ~UTextUUIDComponent() override;

    virtual int CheckRayIntersection(
        FVector& rayOrigin,
        FVector& rayDirection, float& pfNearHitDistance
    ) override;
    void SetUUID(uint32 UUID);
};
