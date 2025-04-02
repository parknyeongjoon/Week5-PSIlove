#include "Material.h"
#include "CoreUObject/UObject/Casts.h"

void UMaterial::DuplicateSubObjects()
{
    // Duplicate에서 다 끝남.
    Super::DuplicateSubObjects();
}

UObject* UMaterial::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UMaterial>(this);

    Cast<UMaterial>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
