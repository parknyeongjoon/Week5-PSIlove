#pragma once
#include "PrimitiveComponent.h"

class UHeightFogComponent : public UPrimitiveComponent
{
    float FogDensity;
    float FogHeightFalloff;
    float StartDistance;
    float FogCutoffDistance;
    float FogMaxOpacity;

    FLinearColor FogInscatteringColor;
};

