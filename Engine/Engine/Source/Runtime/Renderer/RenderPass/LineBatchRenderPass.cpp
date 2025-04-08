#include "LineBatchRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "ViewportClient.h"
#include "Components/LightComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/UObjectIterator.h"

class USceneComponent;
extern FEngineLoop GEngineLoop;

LineBatchRenderPass::LineBatchRenderPass(const FString& InShaderName)
    : BaseRenderPass(InShaderName)
{
    FSimpleVertex vertices[2]{{0}, {0}};
    ID3D11Buffer* pVertexBuffer = FEngineLoop::renderer.CreateStaticVertexBuffer<FSimpleVertex>(vertices, 2);
    FEngineLoop::renderer.AddOrSetVertexBuffer(TEXT("Line"), pVertexBuffer, sizeof(FSimpleVertex), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void LineBatchRenderPass::Prepare(const std::shared_ptr<FViewportClient> viewport)
{
    BaseRenderPass::Prepare(viewport);

    ID3D11ShaderResourceView* BoundingBoxSRV = GEngineLoop.renderer.GetStructuredBufferShaderResourceView(TEXT("BoundingBox"));
    GEngineLoop.graphicDevice.DeviceContext->VSSetShaderResources(2, 1, &BoundingBoxSRV);

    ID3D11ShaderResourceView* ConeSRV = GEngineLoop.renderer.GetStructuredBufferShaderResourceView(TEXT("Cone"));
    GEngineLoop.graphicDevice.DeviceContext->VSSetShaderResources(3, 1, &ConeSRV);

    ID3D11ShaderResourceView* OBBSRV = GEngineLoop.renderer.GetStructuredBufferShaderResourceView(TEXT("OBB"));
    GEngineLoop.graphicDevice.DeviceContext->VSSetShaderResources(4, 1, &OBBSRV);
}

void LineBatchRenderPass::Execute(const std::shared_ptr<FViewportClient> viewport)
{
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(viewport);

    // 쉐이더 내에서 한 번만 Update되어야하는 CB
    const FMatrix Model = FMatrix::Identity;
    FMatrix MVP;
    if (curEditorViewportClient != nullptr)
        MVP = Model * curEditorViewportClient->GetViewMatrix() * curEditorViewportClient->GetProjectionMatrix();
    const FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));

    FMatrixConstants MatrixConstants;
    MatrixConstants.MVP = MVP;
    MatrixConstants.MInverseTranspose = NormalMatrix;
    MatrixConstants.ObjectUUID = FVector4(0,0,0,0);
    MatrixConstants.isSelected = false;
    
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);

    const FGridParametersData GridParameters = UPrimitiveBatch::GetInstance().GetGridParameters();
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FGridParametersData")), &GridParameters);
    
    UpdateBatchResources();

    FPrimitiveCounts PrimitiveCounts;
    PrimitiveCounts.ConeCount = UPrimitiveBatch::GetInstance().GetCones().Num();
    PrimitiveCounts.BoundingBoxCount = UPrimitiveBatch::GetInstance().GetBoundingBoxes().Num();
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FPrimitiveCounts")), &PrimitiveCounts);

    const std::shared_ptr<FVIBuffers> VIBuffer = GEngineLoop.renderer.GetVIBuffer(TEXT("Line"));
    VIBuffer->Bind(GEngineLoop.graphicDevice.DeviceContext);

    UINT vertexCountPerInstance = 2;
    UINT instanceCount = GridParameters.GridCount + 3 + (UPrimitiveBatch::GetInstance().GetBoundingBoxes().Num() * 12) + (UPrimitiveBatch::GetInstance().GetCones().Num() * (2 * UPrimitiveBatch::GetInstance().GetConeSegmentCount()) + (12 * UPrimitiveBatch::GetInstance().GetOrientedBoundingBoxes().Num()));
    GEngineLoop.graphicDevice.DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

    UPrimitiveBatch::GetInstance().ClearBatchPrimitives();
}

void LineBatchRenderPass::AddRenderObjectsToRenderPass(const ULevel* Level)
{
    for (const USceneComponent* iter : TObjectRange<USceneComponent>())
    {
        if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
        {
            FMatrix Model = JungleMath::CreateModelMatrix(pLightComp->GetWorldLocation(), pLightComp->GetWorldRotation(), {1, 1, 1});
            UPrimitiveBatch::GetInstance().AddCone(pLightComp->GetWorldLocation(), pLightComp->GetRadius(), 15, 140, pLightComp->GetColor(), Model);
            UPrimitiveBatch::GetInstance().AddOBB(pLightComp->GetBoundingBox(), pLightComp->GetWorldLocation(), Model);
        }
    }
}

void LineBatchRenderPass::UpdateBatchResources() const
{
    {
        if (UPrimitiveBatch::GetInstance().GetBoundingBoxes().Num() > UPrimitiveBatch::GetInstance().GetAllocatedBoundingBoxCapacity())
        {
            UPrimitiveBatch::GetInstance().SetAllocatedBoundingBoxCapacity(UPrimitiveBatch::GetInstance().GetBoundingBoxes().Num());

            ID3D11Buffer* SB = nullptr;
            ID3D11ShaderResourceView* SBSRV = nullptr;
            SB = FEngineLoop::renderer.CreateStructuredBuffer<FBoundingBox>(static_cast<uint32>(UPrimitiveBatch::GetInstance().GetAllocatedBoundingBoxCapacity()));
            SBSRV = FEngineLoop::renderer.CreateBufferSRV(SB, static_cast<uint32>(UPrimitiveBatch::GetInstance().GetAllocatedBoundingBoxCapacity()));

            FEngineLoop::renderer.AddOrSetStructuredBuffer(TEXT("BoundingBox"), SB);
            FEngineLoop::renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("BoundingBox"), SBSRV);
        }

        ID3D11Buffer* SB = FEngineLoop::renderer.GetStructuredBuffer(TEXT("BoundingBox"));
        ID3D11ShaderResourceView* SBSRV = FEngineLoop::renderer.GetStructuredBufferShaderResourceView(TEXT("BoundingBox"));
        if (SB != nullptr && SBSRV != nullptr)
        {
            FEngineLoop::renderer.UpdateStructuredBuffer(SB, UPrimitiveBatch::GetInstance().GetBoundingBoxes());
        }
    }
    
    {
        if (UPrimitiveBatch::GetInstance().GetCones().Num() > UPrimitiveBatch::GetInstance().GetAllocatedConeCapacity())
        {
            UPrimitiveBatch::GetInstance().SetAllocatedConeCapacity(UPrimitiveBatch::GetInstance().GetCones().Num());

            ID3D11Buffer* SB = nullptr;
            ID3D11ShaderResourceView* SBSRV = nullptr;
            SB = FEngineLoop::renderer.CreateStructuredBuffer<FBoundingBox>(static_cast<UINT>(UPrimitiveBatch::GetInstance().GetAllocatedConeCapacity()));
            SBSRV = FEngineLoop::renderer.CreateBufferSRV(SB, static_cast<UINT>(UPrimitiveBatch::GetInstance().GetAllocatedConeCapacity()));

            FEngineLoop::renderer.AddOrSetStructuredBuffer(TEXT("Cone"), SB);
            FEngineLoop::renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("Cone"), SBSRV);
        }

        ID3D11Buffer* SB = FEngineLoop::renderer.GetStructuredBuffer(TEXT("Cone"));
        ID3D11ShaderResourceView* SBSRV = FEngineLoop::renderer.GetStructuredBufferShaderResourceView(TEXT("Cone"));
        if (SB != nullptr && SBSRV != nullptr)
        {
            FEngineLoop::renderer.UpdateStructuredBuffer(SB, UPrimitiveBatch::GetInstance().GetCones());
        }
    }
    
    {
        if (UPrimitiveBatch::GetInstance().GetOrientedBoundingBoxes().Num() > UPrimitiveBatch::GetInstance().GetAllocatedOBBCapacity())
        {
            UPrimitiveBatch::GetInstance().SetAllocatedOBBCapacity(UPrimitiveBatch::GetInstance().GetOrientedBoundingBoxes().Num());

            ID3D11Buffer* SB = nullptr;
            ID3D11ShaderResourceView* SBSRV = nullptr;
            SB = FEngineLoop::renderer.CreateStructuredBuffer<FBoundingBox>(static_cast<UINT>(UPrimitiveBatch::GetInstance().GetAllocatedOBBCapacity()));
            SBSRV = FEngineLoop::renderer.CreateBufferSRV(SB, static_cast<UINT>(UPrimitiveBatch::GetInstance().GetAllocatedOBBCapacity()));

            FEngineLoop::renderer.AddOrSetStructuredBuffer(TEXT("OBB"), SB);
            FEngineLoop::renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("OBB"), SBSRV);
        }

        ID3D11Buffer* SB = FEngineLoop::renderer.GetStructuredBuffer(TEXT("OBB"));
        ID3D11ShaderResourceView* SBSRV = FEngineLoop::renderer.GetStructuredBufferShaderResourceView(TEXT("OBB"));
        if (SB != nullptr && SBSRV != nullptr)
        {
            FEngineLoop::renderer.UpdateStructuredBuffer(SB, UPrimitiveBatch::GetInstance().GetOrientedBoundingBoxes());
        }
    }
}
