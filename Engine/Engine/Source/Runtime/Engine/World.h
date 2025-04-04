#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UWorld : public UObject
{
    DECLARE_CLASS(UWorld, UObject)
public:
    UWorld() = default;
    ULevel* Level;

    virtual UObject* Duplicate() override;
    void DuplicateSubObjects() override;
};
