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

    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    Graphics.DeviceContext->OMSetDepthStencilState(Renderer.GetDepthStencilState(EDepthStencilState::LessEqual), 0);
    Graphics.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    Graphics.DeviceContext->RSSetState(Renderer.GetRasterizerState(Renderer.GetCurrentRasterizerState()));
    Graphics.DeviceContext->OMSetRenderTargets(5, Graphics.RTVs, Graphics.DepthStencilView); // 렌더 타겟 설정
    Graphics.DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
    
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
}

void StaticMeshRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;

    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    FRenderer& Renderer = GEngineLoop.renderer;
    
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }
    
    for (UStaticMeshComponent* staticMeshComp : StaticMesheComponents)
    {
        const FMatrix Model = JungleMath::CreateModelMatrix(staticMeshComp->GetWorldLocation(), staticMeshComp->GetWorldRotation(),
                                                    staticMeshComp->GetWorldScale());
        
        UpdateMatrixConstants(staticMeshComp, View, Proj);

        UpdateSkySphereTextureConstants(Cast<USkySphereComponent>(staticMeshComp));

        if (curEditorViewportClient->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            if (GEngineLoop.GetLevel()->GetSelectedActor() == staticMeshComp->GetOwner())
            {
                UPrimitiveBatch::GetInstance().AddAABB(
                    staticMeshComp->GetBoundingBox(),
                    staticMeshComp->GetWorldLocation(),
                    Model
                );
            }
        }

        if (!staticMeshComp->GetStaticMesh()) continue;
        
        const OBJ::FStaticMeshRenderData* renderData = staticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        // VIBuffer Bind
        const std::shared_ptr<FVIBuffers> currentVIBuffer =  Renderer.GetVIBuffer(staticMeshComp->VIBufferName);
        currentVIBuffer->Bind(Graphics.DeviceContext);

        // If There's No Material Subset
        if (renderData->MaterialSubsets.Num() == 0)
        {
            Graphics.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0,0);
        }
        const int selectedSubMeshIndex = staticMeshComp->GetselectedSubMeshIndex();

        // SubSet마다 Material Update 및 Draw
        for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); ++subMeshIndex)
        {
            const int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

            const bool bIsSelectedSubMesh = (subMeshIndex == selectedSubMeshIndex);
            UpdateSubMeshConstants(bIsSelectedSubMesh);

            UMaterial* overrideMaterial = staticMeshComp->GetOverrideMaterial(materialIndex);
            if (overrideMaterial != nullptr)
            {
                UpdateMaterialConstants(overrideMaterial->GetMaterialInfo());
            }
            else
            {
                UpdateMaterialConstants(staticMeshComp->GetMaterial(materialIndex)->GetMaterialInfo());
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
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);
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
    
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FUVBuffer")), &UVBuffer);
}

void StaticMeshRenderPass::UpdateSubMeshConstants(const bool bIsSelectedSubMesh)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FSubMeshConstants SubMeshConstants;
    SubMeshConstants.IsSelectedSubMesh = bIsSelectedSubMesh;
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FSubMeshConstants")), &SubMeshConstants);
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
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FMaterialConstants")), &MaterialConstants);
    
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
        Graphics.DeviceContext->PSSetShaderResources(0, 1, nullSRV);
    }
}
