#include "Material.h"
#include "CoreUObject/UObject/Casts.h"

void UMaterial::DuplicateSubObjects()
{
    // Duplicate에서 다 끝남.
}

void UMaterial::Duplicate(const UObject* SourceObject)
{
    Super::Duplicate(SourceObject);

    if (UMaterial* SourceMaterial = Cast<UMaterial>(SourceObject))
    {
        this->materialInfo = SourceMaterial->materialInfo;
    }
}
