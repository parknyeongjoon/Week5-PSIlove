#pragma once
#include "Components/ActorComponent.h"

class UProjectileMovementComponent : public UActorComponent
{
    DECLARE_CLASS(UProjectileMovementComponent, UActorComponent)

public:
    UPROPERTY(FVector, Velocity)
    UPROPERTY(FVector, MaxSpeed)
    UPROPERTY(FVector, Acceleration)

    UObject* Duplicate() override;
private:
    void TickComponent(float DeltaTime) override;
    
    FVector Velocity;
    FVector MaxSpeed;
    FVector Acceleration;
};
