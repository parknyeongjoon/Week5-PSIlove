#pragma once
#include "Engine/StaticMeshActor.h"

class AFireBall : public AStaticMeshActor
{
    DECLARE_CLASS(AFireBall, AStaticMeshActor)
public:
    AFireBall();
    void Tick(float DeltaTime) override;
    UObject* Duplicate() override;
};
