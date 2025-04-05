#include "StaticMesh.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/ObjectFactory.h"
#include "CoreUObject/UObject/Casts.h"

UStaticMesh::UStaticMesh()
{

}

UStaticMesh::~UStaticMesh()
{
    if (staticMeshRenderData == nullptr) return;

    if (staticMeshRenderData->VertexBuffer) {
        staticMeshRenderData->VertexBuffer->Release();
        staticMeshRenderData->VertexBuffer = nullptr;
    }

    if (staticMeshRenderData->IndexBuffer) {
        staticMeshRenderData->IndexBuffer->Release();
        staticMeshRenderData->IndexBuffer = nullptr;
    }
}

void UStaticMesh::DuplicateSubObjects()
{
    // FStaticMaterial이 UMaterial*을 갖고 있긴 하지만, shallow copy만 필요하기 때문에 별도로 신경 쓸필요 없음.
    // UMaterial을 복사해서 변경하는건 다른데서 해야함...
    // 만약 material을 변경하면 동일한 material을 가진 다른 오브젝트에게도 영향이 감.
    Super::DuplicateSubObjects();
}

UObject* UStaticMesh::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UStaticMesh>(this);

    Cast<UStaticMesh>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

uint32 UStaticMesh::GetMaterialIndex(FName MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < materials.Num(); materialIndex++) {
        if (materials[materialIndex]->MaterialSlotName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void UStaticMesh::GetUsedMaterials(TArray<UMaterial*>& Out) const
{
    for (const FStaticMaterial* Material : materials)
    {
        Out.Emplace(Material->Material);
    }
}

void UStaticMesh::SetData(OBJ::FStaticMeshRenderData* renderData)
{
    staticMeshRenderData = renderData;

    uint32 verticeNum = staticMeshRenderData->Vertices.Num();
    if (verticeNum <= 0) return;
    
    staticMeshRenderData->VertexBuffer = GetEngine().renderer.CreateImmutableVertexBuffer<FVertexSimple>(staticMeshRenderData->Vertices);
    GetEngine().renderer.AddOrSetVertexBuffer(staticMeshRenderData->DisplayName, staticMeshRenderData->VertexBuffer, sizeof(FVertexSimple));
    
    uint32 indexNum = staticMeshRenderData->Indices.Num();
    if (indexNum > 0)
        staticMeshRenderData->IndexBuffer = GetEngine().renderer.CreateIndexBuffer(staticMeshRenderData->Indices);
    GetEngine().renderer.AddOrSetIndexBuffer(staticMeshRenderData->DisplayName, staticMeshRenderData->IndexBuffer, indexNum);

    for (int materialIndex = 0; materialIndex < staticMeshRenderData->Materials.Num(); materialIndex++) {
        FStaticMaterial* newMaterialSlot = new FStaticMaterial();
        UMaterial* newMaterial = FManagerOBJ::CreateMaterial(staticMeshRenderData->Materials[materialIndex]);

        newMaterialSlot->Material = newMaterial;
        newMaterialSlot->MaterialSlotName = staticMeshRenderData->Materials[materialIndex].MTLName;

        materials.Add(newMaterialSlot);
    }
}
