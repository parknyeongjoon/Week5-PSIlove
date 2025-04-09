#pragma once
#include "PrimitiveComponent.h"
#include <UObject/ObjectMacros.h>

class UHeightFogComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UHeightFogComponent, UPrimitiveComponent)
public:
    UHeightFogComponent();
    virtual ~UHeightFogComponent() override;

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    float FogDensity = 0.1f;
    float FogHeightFalloff = 0.5f;
    float StartDistance = 100.0f;
    float FogCutoffDistance = 5000.0f;
    float FogMaxOpacity = 0.7f;

    FLinearColor FogInscatteringColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    float GetFogCutoffDistance() const{ return FogCutoffDistance; }
    void SetFogCutoffDistance(float distance){ FogCutoffDistance = distance; }
};

