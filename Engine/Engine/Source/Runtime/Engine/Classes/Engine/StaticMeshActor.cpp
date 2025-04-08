#include "StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"


AStaticMeshActor::AStaticMeshActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>();
    RootComponent = StaticMeshComponent;
}

UObject* AStaticMeshActor::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<AStaticMeshActor>(this);

    Cast<AStaticMeshActor>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
