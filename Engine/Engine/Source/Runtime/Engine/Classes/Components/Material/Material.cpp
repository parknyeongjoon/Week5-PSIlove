#include "Material.h"
#include "CoreUObject/UObject/Casts.h"

void UMaterial::DuplicateSubObjects()
{
    // DuplicateObject에서 다 끝남.
}

void UMaterial::DuplicateObject(const UObject* SourceObject)
{
    if (UMaterial* SourceMaterial = Cast<UMaterial>(SourceObject))
    {
        this->materialInfo = SourceMaterial->materialInfo;
    }
}
