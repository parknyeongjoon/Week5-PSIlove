#include "StaticMeshRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "ViewportClient.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SkySphereComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"
#include "PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"

extern FEngineLoop GEngineLoop;

void StaticMeshRenderPass::Prepare(const std::shared_ptr<FViewportClient> viewport)
{
    BaseRenderPass::Prepare(viewport);
}

void StaticMeshRenderPass::Execute(const std::shared_ptr<FViewportClient> viewport)
{
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;
    
    // 쉐이더 내에서 한 번만 Update되어야하는 정보
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(viewport);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }
    
    for (const auto item : StaticMesheComponents)
    {
        const FMatrix Model = JungleMath::CreateModelMatrix(item->GetWorldLocation(), item->GetWorldRotation(),
                                                    item->GetWorldScale());
        
        UpdateMatrixConstants(item, View, Proj);

        UpdateSkySphereTextureConstants(Cast<USkySphereComponent>(item));

        if (curEditorViewportClient->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::Type::SF_AABB))
        {
            UPrimitiveBatch::GetInstance().AddAABB(
                item->GetBoundingBox(),
                item->GetWorldLocation(),
                Model
            );
        }

        if (!item->GetStaticMesh()) continue;
        
        // VIBuffer Bind
        const std::shared_ptr<FVIBuffers> currentVIBuffer =  GEngineLoop.renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(GEngineLoop.graphicDevice.DeviceContext);
        
        const OBJ::FStaticMeshRenderData* renderData = item->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        // If There's No Material Subset
        if (renderData->MaterialSubsets.Num() == 0)
        {
            GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0,0);
        }
        const int selectedSubMeshIndex = item->GetselectedSubMeshIndex();

        // SubSet마다 Material Update 및 Draw
        for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); subMeshIndex++)
        {
            const int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

            const bool bIsSelectedSubMesh = (subMeshIndex == selectedSubMeshIndex);
            UpdateSubMeshConstants(bIsSelectedSubMesh);

            UMaterial* overrideMaterial = item->GetOverrideMaterial(materialIndex);
            if (overrideMaterial)
            {
                UpdateMaterialConstants(overrideMaterial->GetMaterialInfo());
            }
            else
            {
                UpdateMaterialConstants(item->GetMaterial(materialIndex)->GetMaterialInfo());
            }
            
            if (currentVIBuffer != nullptr)
            {
                // index draw
                const uint64 startIndex = renderData->MaterialSubsets[subMeshIndex].IndexStart;
                const uint64 indexCount = renderData->MaterialSubsets[subMeshIndex].IndexCount;
                GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(indexCount, startIndex, 0);
            }
        }
    }
}

void StaticMeshRenderPass::AddRenderObjectsToRenderPass(const ULevel* Level)
{
    StaticMesheComponents.Empty();
    TArray<USceneComponent*> Ss;
    for (const auto& A : Level->GetActors())
    {
        Ss.Add(A->GetRootComponent());
        TArray<USceneComponent*> temp;
        A->GetRootComponent()->GetChildrenComponents(temp);
        Ss + temp;
    }

    for (const auto iter : Ss)
    {
        if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
        {
            if (Cast<UGizmoBaseComponent>(iter))
            {
                continue;
            }
            StaticMesheComponents.Add(pStaticMeshComp);
        }
    }
}

void StaticMeshRenderPass::UpdateMatrixConstants(UStaticMeshComponent* InStaticMeshComponent, const FMatrix& InView, const FMatrix& InProjection)
{
    // MVP Update
    const FMatrix Model = JungleMath::CreateModelMatrix(InStaticMeshComponent->GetWorldLocation(), InStaticMeshComponent->GetWorldRotation(),
                                                        InStaticMeshComponent->GetWorldScale());
    const FMatrix MVP = Model * InView * InProjection;
    const FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    const FVector4 UUIDColor = InStaticMeshComponent->EncodeUUID() / 255.0f;
        
    FMatrixConstants MatrixConstants;
    MatrixConstants.MVP = MVP;
    MatrixConstants.MInverseTranspose = NormalMatrix;
    MatrixConstants.ObjectUUID = UUIDColor;
    if (InStaticMeshComponent->GetLevel()->GetSelectedActor() == InStaticMeshComponent->GetOwner())
    {
        MatrixConstants.isSelected = true;
    }
    else
    {
        MatrixConstants.isSelected = false;
    }
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);
}

void StaticMeshRenderPass::UpdateSkySphereTextureConstants(const USkySphereComponent* InSkySphereComponent)
{
    FTextureConstants TextureConstants;
    if (InSkySphereComponent != nullptr)
    {
        TextureConstants.UVOffset = FVector2D(InSkySphereComponent->UOffset, InSkySphereComponent->VOffset);
    }
    else
    {
        TextureConstants.UVOffset = FVector2D(0.0f, 0.0f);
    }
    
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FTextureConstants")), &TextureConstants);
}

void StaticMeshRenderPass::UpdateSubMeshConstants(const bool bIsSelectedSubMesh)
{
    FSubMeshConstants SubMeshConstants;
    SubMeshConstants.IsSelectedSubMesh = bIsSelectedSubMesh;
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FSubMeshConstants")), &SubMeshConstants);
}

void StaticMeshRenderPass::UpdateMaterialConstants(const FObjMaterialInfo& MaterialInfo)
{
    FMaterialConstants MaterialConstants;
    MaterialConstants.DiffuseColor = MaterialInfo.Diffuse;
    MaterialConstants.TransparencyScalar = MaterialInfo.TransparencyScalar;
    MaterialConstants.AmbientColor = MaterialInfo.Ambient;
    MaterialConstants.DensityScalar = MaterialInfo.DensityScalar;
    MaterialConstants.SpecularColor = MaterialInfo.Specular;
    MaterialConstants.SpecularScalar = MaterialInfo.SpecularScalar;
    MaterialConstants.EmissiveColor = MaterialInfo.Emissive;
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FMaterialConstants")), &MaterialConstants);
    
    if (MaterialInfo.bHasTexture == true)
    {
        std::shared_ptr<FTexture> texture = GEngineLoop.resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
        GEngineLoop.graphicDevice.DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
        ID3D11SamplerState* linearSampler = GEngineLoop.renderer.GetSamplerState(ESamplerType::Linear);
        GEngineLoop.graphicDevice.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        ID3D11SamplerState* nullSampler[1] = {nullptr};

        GEngineLoop.graphicDevice.DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        GEngineLoop.graphicDevice.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, nullSampler);
    }
}
