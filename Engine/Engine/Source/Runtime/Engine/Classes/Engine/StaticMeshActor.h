#pragma once
#include "GameFramework/Actor.h"


class AStaticMeshActor : public AActor
{
    DECLARE_CLASS(AStaticMeshActor, AActor)

public:
    AStaticMeshActor();

    virtual void DuplicateSubObjects();
    virtual void DuplicateObject(const UObject* SourceObject);

public:
    UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

private:
    UStaticMeshComponent* StaticMeshComponent = nullptr;
};
