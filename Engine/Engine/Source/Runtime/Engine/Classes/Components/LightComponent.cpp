#include "LightComponent.h"

ULightComponent::ULightComponent()
{
    InitializeLight();
}

void ULightComponent::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
}
UObject* ULightComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<ULightComponent>(this);

    Cast<ULightComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void ULightComponent::InitializeLight()
{
    AABB.max = { 1.f,1.f,1.f };
    AABB.min = { -1.f,-1.f,-1.f };
    Color = { 1,1,1, 1 };
    AttenuationRadius = 5;
}

void ULightComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

int ULightComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
}

