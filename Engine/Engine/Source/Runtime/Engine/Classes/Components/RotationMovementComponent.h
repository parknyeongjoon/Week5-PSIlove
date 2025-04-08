#pragma once
#include "ActorComponent.h"

class URotationMovementComponent : public UActorComponent
{
    DECLARE_CLASS(URotationMovementComponent, UActorComponent)
public:
    UPROPERTY(FVector, RotationRate)
    UObject* Duplicate() override;
private:
    void TickComponent(float DeltaTime) override;
    FVector RotationRate = FVector(0.01, 0.01, 0.01);
};
