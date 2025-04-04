#include "MeshComponent.h"


void UMeshComponent::DuplicateSubObjects()
{
    // 어떤걸로 override되어있는지는 복사가 되지만,
    // override가 된 UMaterial 자체는 복사가 되지 않음
    Super::DuplicateSubObjects();
}

UObject* UMeshComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UMeshComponent>(this);

    Cast<UMeshComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

UMaterial* UMeshComponent::GetMaterial(uint32 ElementIndex) const
{
    if (OverrideMaterials.IsValidIndex(ElementIndex))
        return OverrideMaterials[ElementIndex];

    return nullptr;
}

uint32 UMeshComponent::GetMaterialIndex(FName MaterialSlotName) const
{
    // This function should be overridden
    return INDEX_NONE;
}

UMaterial* UMeshComponent::GetMaterialByName(FName MaterialSlotName) const
{
    int32 MaterialIndex = GetMaterialIndex(MaterialSlotName);
    if (MaterialIndex < 0)
        return nullptr;
    return GetMaterial(MaterialIndex);
}

TArray<FName> UMeshComponent::GetMaterialSlotNames() const
{
    return TArray<FName>();
}

void UMeshComponent::SetMaterial(uint32 ElementIndex, UMaterial* Material)
{
    if (OverrideMaterials.IsValidIndex(ElementIndex) == false) return;

    OverrideMaterials[ElementIndex] = Material;
}

void UMeshComponent::SetMaterialByName(FName MaterialSlotName, UMaterial* Material)
{
    int32 MaterialIndex = GetMaterialIndex(MaterialSlotName);
    if (MaterialIndex < 0)
        return;

    SetMaterial(MaterialIndex, Material);
}

void UMeshComponent::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (int32 ElementIndex = 0; ElementIndex < GetNumMaterials(); ElementIndex++)
    {
        if (UMaterial* Material = GetMaterial(ElementIndex))
        {
            Out.Add(Material);
        }
    }
}
