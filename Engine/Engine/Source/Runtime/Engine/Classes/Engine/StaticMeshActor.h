#pragma once
#include "GameFramework/Actor.h"


class AStaticMeshActor : public AActor
{
    DECLARE_CLASS(AStaticMeshActor, AActor)

public:
    AStaticMeshActor();

    virtual void DuplicateSubObjects();
    virtual void Duplicate(const UObject* SourceObject);

public:
    UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

private:
    UStaticMeshComponent* StaticMeshComponent = nullptr;
};
