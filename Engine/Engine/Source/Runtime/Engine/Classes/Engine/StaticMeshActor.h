#pragma once
#include "GameFramework/Actor.h"


class AStaticMeshActor : public AActor
{
    DECLARE_CLASS(AStaticMeshActor, AActor)

public:
    AStaticMeshActor();

    virtual UObject* Duplicate() override;
    
    UStaticMeshComponent* GetStaticMeshComponent() const { return StaticMeshComponent; }

protected:
    UStaticMeshComponent* StaticMeshComponent = nullptr;
};
