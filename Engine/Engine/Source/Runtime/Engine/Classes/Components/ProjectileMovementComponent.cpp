#include "ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"

UObject* UProjectileMovementComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UProjectileMovementComponent>(this);

    Cast<UProjectileMovementComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UProjectileMovementComponent::TickComponent(float DeltaTime)
{
    GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + Velocity * DeltaTime);
}
