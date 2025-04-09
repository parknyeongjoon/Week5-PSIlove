#include "HeightFogComponent.h"

UHeightFogComponent::UHeightFogComponent()
{
    SetType(StaticClass()->GetName());
}

UHeightFogComponent::~UHeightFogComponent()
{
}

void UHeightFogComponent::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
}

UObject* UHeightFogComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UHeightFogComponent>(this);

    Cast<UHeightFogComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UHeightFogComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UHeightFogComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
