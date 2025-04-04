#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

void AStaticMeshActor::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
    this->StaticMeshComponent = Cast<UStaticMeshComponent>(this->StaticMeshComponent->Duplicate());
}

UObject* AStaticMeshActor::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<AStaticMeshActor>(this);

    Cast<AStaticMeshActor>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
