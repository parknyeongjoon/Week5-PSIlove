#include "ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"

void UProjectileMovementComponent::AddVelocity(const FVector& value)
{
    Velocity = Velocity + value;
    if (Velocity.Magnitude() > MaxSpeed)
    {
        Velocity = Velocity.Normalize() * MaxSpeed;
    }
}

UObject* UProjectileMovementComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UProjectileMovementComponent>(this);

    Cast<UProjectileMovementComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + Velocity * DeltaTime);
    Velocity = Velocity * 0.95f;
}
