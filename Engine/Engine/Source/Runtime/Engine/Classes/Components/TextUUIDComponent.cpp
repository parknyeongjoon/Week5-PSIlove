#include "TextUUIDComponent.h"

UTextUUIDComponent::UTextUUIDComponent()
{
    SetScale(FVector(0.1f, 0.25f, 0.25f));
    SetLocation(FVector(0.0f, 0.0f, 1.5f));
    SetTexture(L"Assets/Texture/UUID_Font.png");
    SetRowColumnCount(1, 11);
}

UTextUUIDComponent::~UTextUUIDComponent()
{
}

int UTextUUIDComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return 0;
}

void UTextUUIDComponent::SetUUID(uint32 UUID)
{
    SetText(std::to_wstring(UUID));
}


