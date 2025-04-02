#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UWorld : public UObject
{
    DECLARE_CLASS(UWorld, UObject)
public:
    ULevel* Level;

    void WorldTick(float DeltaTime);

    void DuplicateObject(const UObject* SourceObject) override;
    void DuplicateSubObjects() override;
};
