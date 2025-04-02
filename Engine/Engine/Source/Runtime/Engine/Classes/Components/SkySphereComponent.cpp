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
    Super::DuplicateSubObjects();
}

UObject* USkySphereComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<USkySphereComponent>(this);

    Cast<USkySphereComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
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