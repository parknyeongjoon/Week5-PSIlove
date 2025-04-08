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

void GizmoRenderPass::Prepare(const std::shared_ptr<FViewportClient> viewport)
{
    BaseRenderPass::Prepare(viewport);
    GEngineLoop.graphicDevice.DeviceContext->RSSetState(GEngineLoop.renderer.GetRasterizerState(ERasterizerState::SolidBack)); //레스터 라이저 상태 설정
}

void GizmoRenderPass::Execute(const std::shared_ptr<FViewportClient> viewport)
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
        
            // 재질 상수 버퍼 업데이트
            const UMaterial* CurrentMaterial = item->GetMaterial(materialIndex);
            UpdateMaterialConstants(CurrentMaterial);
            
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

void GizmoRenderPass::AddRenderObjectsToRenderPass(const ULevel* Level)
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
    // MVP Update
    const FMatrix Model = JungleMath::CreateModelMatrix(InGizmoComponent->GetWorldLocation(), InGizmoComponent->GetWorldRotation(),
                                                        InGizmoComponent->GetWorldScale());
    const FMatrix MVP = Model * InView * InProjection;
    const FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    const FVector4 UUIDColor = InGizmoComponent->EncodeUUID() / 255.0f;
        
    FMatrixConstants MatrixConstants;
    MatrixConstants.MVP = MVP;
    MatrixConstants.MInverseTranspose = NormalMatrix;
    MatrixConstants.ObjectUUID = UUIDColor;
    if (InGizmoComponent->GetLevel()->GetPickingGizmo() == InGizmoComponent)
    {
        MatrixConstants.isSelected = true;
    }
    else
    {
        MatrixConstants.isSelected = false;
    }
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);
}

void GizmoRenderPass::UpdateSubMeshConstants(bool bIsSelectedSubMesh)
{
    FSubMeshConstants SubMeshConstants;
    SubMeshConstants.IsSelectedSubMesh = bIsSelectedSubMesh;
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FSubMeshConstants")), &SubMeshConstants);
}

void GizmoRenderPass::UpdateMaterialConstants(const UMaterial* CurrentMaterial)
{
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
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FMaterialConstants")), &MaterialConstants);
}
