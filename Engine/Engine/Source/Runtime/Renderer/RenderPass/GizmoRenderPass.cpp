#include "GizmoRenderPass.h"

#include "Level.h"
#include "Actors/Player.h"
#include "Math/JungleMath.h"
#include "Math/Matrix.h"
#include "UnrealEd/EditorViewportClient.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "UObject/ObjectTypes.h"
#include "UObject/UObjectIterator.h"

void GizmoRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
    
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    Graphics.DeviceContext->OMSetDepthStencilState(Renderer.GetDepthStencilState(EDepthStencilState::DepthNone), 0);
    Graphics.DeviceContext->RSSetState(Renderer.GetRasterizerState(ERasterizerState::SolidBack)); //레스터 라이저 상태 설정
}

void GizmoRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;
    
    // 쉐이더 내에서 한 번만 Update되어야하는 정보
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }

    for (const auto item : GizmoComponents)
    {
        if ((item->GetGizmoType()==UGizmoBaseComponent::ArrowX ||
            item->GetGizmoType()==UGizmoBaseComponent::ArrowY ||
            item->GetGizmoType()==UGizmoBaseComponent::ArrowZ)
            && item->GetLevel()->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((item->GetGizmoType()==UGizmoBaseComponent::ScaleX ||
            item->GetGizmoType()==UGizmoBaseComponent::ScaleY ||
            item->GetGizmoType()==UGizmoBaseComponent::ScaleZ)
            && item->GetLevel()->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((item->GetGizmoType()==UGizmoBaseComponent::CircleX ||
            item->GetGizmoType()==UGizmoBaseComponent::CircleY ||
            item->GetGizmoType()==UGizmoBaseComponent::CircleZ)
            && item->GetLevel()->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;

        UpdateMatrixConstants(item, View, Proj);

        if (!item->GetStaticMesh()) continue;

        const OBJ::FStaticMeshRenderData* renderData = item->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        const std::shared_ptr<FVIBuffers> currentVIBuffer =  Renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(Graphics.DeviceContext);

        // If There's No Material Subset
        if (renderData->MaterialSubsets.Num() == 0)
        {
            Graphics.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0,0);
        }
        
        // SubSet마다 Material Update 및 Draw
        for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); subMeshIndex++)
        {
            const int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;
        
            const bool bIsSelectedSubMesh = (subMeshIndex == -1);
            UpdateSubMeshConstants(bIsSelectedSubMesh);
        
            // 재질 상수 버퍼 업데이트
            const UMaterial* CurrentMaterial = item->GetMaterial(materialIndex);
            UpdateMaterialConstants(CurrentMaterial);
            
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

void GizmoRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    GizmoComponents.Empty();
    for (const USceneComponent* iter : TObjectRange<USceneComponent>())
    {
        if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
        {
            GizmoComponents.Add(pGizmoComp);
        }
    }
}

void GizmoRenderPass::UpdateMatrixConstants(UGizmoBaseComponent* InGizmoComponent, const FMatrix& InView, const FMatrix& InProjection)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    
    // MVP Update
    const FMatrix Model = JungleMath::CreateModelMatrix(InGizmoComponent->GetWorldLocation(), InGizmoComponent->GetWorldRotation(),
                                                        InGizmoComponent->GetWorldScale());
    const FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        
    FMatrixConstants MatrixConstants;
    MatrixConstants.M = Model;
    MatrixConstants.VP = InView * InProjection;
    MatrixConstants.MInverseTranspose = NormalMatrix;
    if (InGizmoComponent->GetLevel()->GetPickingGizmo() == InGizmoComponent)
    {
        MatrixConstants.isSelected = true;
    }
    else
    {
        MatrixConstants.isSelected = false;
    }
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);
}

void GizmoRenderPass::UpdateSubMeshConstants(bool bIsSelectedSubMesh)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    
    FSubMeshConstants SubMeshConstants;
    SubMeshConstants.IsSelectedSubMesh = bIsSelectedSubMesh;
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FSubMeshConstants")), &SubMeshConstants);
}

void GizmoRenderPass::UpdateMaterialConstants(const UMaterial* CurrentMaterial)
{
    FRenderer& Renderer = GEngineLoop.renderer;

    FMaterialConstants MaterialConstants;
    if (CurrentMaterial != nullptr)
    {
        MaterialConstants.DiffuseColor = CurrentMaterial->GetDiffuse();
        MaterialConstants.TransparencyScalar = CurrentMaterial->GetTransparency();
        MaterialConstants.AmbientColor = CurrentMaterial->GetAmbient();
        MaterialConstants.DensityScalar = CurrentMaterial->GetDensity();
        MaterialConstants.SpecularColor = CurrentMaterial->GetSpecular();
        MaterialConstants.SpecularScalar = CurrentMaterial->GetSpecularScalar();
        MaterialConstants.EmissiveColor = CurrentMaterial->GetEmissive();
    }
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FMaterialConstants")), &MaterialConstants);
}
