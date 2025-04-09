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

void StaticMeshRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
}

void StaticMeshRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;

    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    FRenderer& Renderer = GEngineLoop.renderer;

    // 쉐이더 내에서 한 번만 Update되어야하는 정보
    FFlagConstants flagConstants;
    flagConstants.IsLit = Renderer.IsLit();
    Renderer.UpdateConstnatBuffer<FFlagConstants>(Renderer.GetConstantBuffer(TEXT("FFlagConstants")), &flagConstants);
    
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
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
        const std::shared_ptr<FVIBuffers> currentVIBuffer =  Renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(Graphics.DeviceContext);
        
        const OBJ::FStaticMeshRenderData* renderData = item->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        // If There's No Material Subset
        if (renderData->MaterialSubsets.Num() == 0)
        {
            Graphics.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0,0);
        }
        const int selectedSubMeshIndex = item->GetselectedSubMeshIndex();

        // SubSet마다 Material Update 및 Draw
        for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); ++subMeshIndex)
        {
            const int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

            const bool bIsSelectedSubMesh = (subMeshIndex == selectedSubMeshIndex);
            UpdateSubMeshConstants(bIsSelectedSubMesh);

            UMaterial* overrideMaterial = item->GetOverrideMaterial(materialIndex);
            if (overrideMaterial != nullptr)
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
                Graphics.DeviceContext->DrawIndexed(indexCount, startIndex, 0);
            }
        }
    }
}

void StaticMeshRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    StaticMesheComponents.Empty();
    TArray<USceneComponent*> Ss;
    for (const auto& A : InLevel->GetActors())
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
    FRenderer& Renderer = GEngineLoop.renderer;
    // MVP Update
    const FMatrix Model = JungleMath::CreateModelMatrix(InStaticMeshComponent->GetWorldLocation(), InStaticMeshComponent->GetWorldRotation(),
                                                        InStaticMeshComponent->GetWorldScale());
    const FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        
    FMatrixConstants MatrixConstants;
    MatrixConstants.M = Model;
    MatrixConstants.VP = InView * InProjection;
    MatrixConstants.MInverseTranspose = NormalMatrix;
    if (InStaticMeshComponent->GetLevel()->GetSelectedActor() == InStaticMeshComponent->GetOwner())
    {
        MatrixConstants.isSelected = true;
    }
    else
    {
        MatrixConstants.isSelected = false;
    }
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);
}

void StaticMeshRenderPass::UpdateSkySphereTextureConstants(const USkySphereComponent* InSkySphereComponent)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FUVBuffer UVBuffer;
    
    if (InSkySphereComponent != nullptr)
    {
        UVBuffer.UOffset = InSkySphereComponent->UOffset;
        UVBuffer.VOffset = InSkySphereComponent->VOffset;
        UVBuffer.UTiles = 1;
        UVBuffer.VTiles = 1;
    }
    else
    {
        UVBuffer.UOffset = 0;
        UVBuffer.VOffset = 0;
        UVBuffer.UTiles = 1;
        UVBuffer.VTiles = 1;
    }
    
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FUVBuffer")), &UVBuffer);
}

void StaticMeshRenderPass::UpdateSubMeshConstants(const bool bIsSelectedSubMesh)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FSubMeshConstants SubMeshConstants;
    SubMeshConstants.IsSelectedSubMesh = bIsSelectedSubMesh;
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FSubMeshConstants")), &SubMeshConstants);
}

void StaticMeshRenderPass::UpdateMaterialConstants(const FObjMaterialInfo& MaterialInfo)
{
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    FRenderer& Renderer = GEngineLoop.renderer;
    
    FMaterialConstants MaterialConstants;
    MaterialConstants.DiffuseColor = MaterialInfo.Diffuse;
    MaterialConstants.TransparencyScalar = MaterialInfo.TransparencyScalar;
    MaterialConstants.AmbientColor = MaterialInfo.Ambient;
    MaterialConstants.DensityScalar = MaterialInfo.DensityScalar;
    MaterialConstants.SpecularColor = MaterialInfo.Specular;
    MaterialConstants.SpecularScalar = MaterialInfo.SpecularScalar;
    MaterialConstants.EmissiveColor = MaterialInfo.Emissive;
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FMaterialConstants")), &MaterialConstants);
    
    if (MaterialInfo.bHasTexture == true)
    {
        std::shared_ptr<FTexture> texture = GEngineLoop.resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
        Graphics.DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
        ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
        Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        ID3D11SamplerState* nullSampler[1] = {nullptr};

        Graphics.DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, nullSampler);
    }
}
