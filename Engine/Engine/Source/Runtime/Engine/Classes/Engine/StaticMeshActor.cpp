#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

void AStaticMeshActor::DuplicateSubObjects()
{
    this->StaticMeshComponent = this->StaticMeshComponent->Duplicate<UStaticMeshComponent>();
}

void AStaticMeshActor::Duplicate(const UObject* SourceObject)
{
    Super::Duplicate(SourceObject);

    if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(SourceObject))
    {
        this->StaticMeshComponent = StaticMeshActor->StaticMeshComponent;
    }
}
