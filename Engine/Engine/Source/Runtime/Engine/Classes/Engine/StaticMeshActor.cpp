#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

void AStaticMeshActor::DuplicateSubObjects()
{
}

void AStaticMeshActor::DuplicateObject(const UObject* SourceObject)
{
    if (AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(SourceObject))
    {
        this->StaticMeshComponent = StaticMeshActor->StaticMeshComponent;
    }
}
