#pragma once
#include "Components/ActorComponent.h"

class UProjectileMovementComponent : public UActorComponent
{
    DECLARE_CLASS(UProjectileMovementComponent, UActorComponent)

public:
    UPROPERTY(FVector, Velocity)
    UPROPERTY(float, MaxSpeed)
    UPROPERTY(float, Acceleration)

    void AddVelocity(const FVector& value);

    UObject* Duplicate() override;
private:
    void TickComponent(float DeltaTime) override;
    
    FVector Velocity;
    float MaxSpeed = 3.0f;
    float Acceleration = 0.1f;
};
