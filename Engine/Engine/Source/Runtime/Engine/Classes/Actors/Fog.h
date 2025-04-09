#pragma once
#include "GameFramework/Actor.h"

class AFog : public AActor
{
    DECLARE_CLASS(AFog, AActor)

    AFog();
    ~AFog();

    virtual void Tick(float DeltaTime) override;
    UObject* Duplicate() override;
};

