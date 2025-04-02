#include "LightComponent.h"
#include "UBillboardComponent.h"
#include "Math/JungleMath.h"
#include "UnrealEd/PrimitiveBatch.h"

ULightComponentBase::ULightComponentBase()
{
    // FString name = "SpotLight";
    // SetName(name);
    InitializeLight();
}

ULightComponentBase::~ULightComponentBase()
{
    delete texture2D;
}
void ULightComponentBase::DuplicateSubObjects()
{
    this->texture2D = this->texture2D->Duplicate<UBillboardComponent>();
}
void ULightComponentBase::DuplicateObject(const UObject* SourceObject)
{
    if (ULightComponentBase* LightComponentBase = Cast<ULightComponentBase>(SourceObject))
    {
        this->color = LightComponentBase->color;
        this->radius = LightComponentBase->radius;
        this->AABB = LightComponentBase->AABB;
        this->texture2D = LightComponentBase->texture2D;
    }
}
void ULightComponentBase::SetColor(FVector4 newColor)
{
    color = newColor;
}

FVector4 ULightComponentBase::GetColor() const
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
    texture2D = new UBillboardComponent();
    texture2D->SetTexture(L"Assets/Texture/spotLight.png");
    texture2D->InitializeComponent();
    AABB.max = { 1.f,1.f,0.1f };
    AABB.min = { -1.f,-1.f,-0.1f };
    color = { 1,1,1,1 };
    radius = 5;
}

void ULightComponentBase::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    texture2D->TickComponent(DeltaTime);
    texture2D->SetLocation(GetWorldLocation());

}

int ULightComponentBase::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    bool res =AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance);
    return res;
}

