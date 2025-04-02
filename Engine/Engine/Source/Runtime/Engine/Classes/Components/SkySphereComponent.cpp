#include "SkySphereComponent.h"

#include "Level.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "LevelEditor/SLevelEditor.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"


USkySphereComponent::USkySphereComponent()
{
    SetType(StaticClass()->GetName());
}

USkySphereComponent::~USkySphereComponent()
{
}

void USkySphereComponent::DuplicateSubObjects()
{
    // deepcopy 대상 없음.
}

void USkySphereComponent::DuplicateObject(const UObject* SourceObject)
{
    if (USkySphereComponent* SkySphereComponent = Cast<USkySphereComponent>(SourceObject))
    {
        this->UOffset = SkySphereComponent->UOffset;
        this->VOffset = SkySphereComponent->VOffset;
    }
}

void USkySphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USkySphereComponent::TickComponent(float DeltaTime)
{
    UOffset += 0.005f;
    VOffset += 0.005f;
    Super::TickComponent(DeltaTime);
}