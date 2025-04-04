#include "LightComponent.h"

ULightComponentBase::ULightComponentBase()
{
    InitializeLight();
}

void ULightComponentBase::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
}
UObject* ULightComponentBase::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<ULightComponentBase>(this);

    Cast<ULightComponentBase>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
void ULightComponentBase::SetColor(FLinearColor newColor)
{
    color = newColor;
}

FLinearColor ULightComponentBase::GetColor() const
{
    return color;
}

float ULightComponentBase::GetRadius() const
{
    return radius;
}

void ULightComponentBase::SetRadius(float r)
{
    radius = r;
}

void ULightComponentBase::InitializeLight()
{
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    color = { 1,1,1, 1 };
    radius = 5;
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
}

