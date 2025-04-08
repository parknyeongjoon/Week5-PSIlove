#include "RotationMovementComponent.h"

#include "GameFramework/Actor.h"

UObject* URotationMovementComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<URotationMovementComponent>(this);

    Cast<URotationMovementComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void URotationMovementComponent::TickComponent(float DeltaTime)
{
    GetOwner()->SetActorRotation(GetOwner()->GetActorRotation() + RotationRate * DeltaTime);
}
