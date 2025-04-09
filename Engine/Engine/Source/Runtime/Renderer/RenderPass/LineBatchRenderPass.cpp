#include "LineBatchRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "ViewportClient.h"
#include "Components/LightComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"
#include "PropertyEditor/ShowFlags.h"
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
    FEngineLoop::renderer.AddOrSetVertexBuffer(TEXT("Line"), pVertexBuffer, sizeof(FSimpleVertex), 2, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}

void LineBatchRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
    
    FRenderer& Renderer = GEngineLoop.renderer;
    const FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    Graphics.DeviceContext->RSSetState(Renderer.GetRasterizerState(ERasterizerState::SolidBack));

    auto* RenderTarget = Graphics.GetWriteRTV();
    Graphics.DeviceContext->OMSetDepthStencilState(Renderer.GetDepthStencilState(EDepthStencilState::LessEqual), 0);
    Graphics.DeviceContext->OMSetRenderTargets(1, &RenderTarget, Graphics.pingpongDSV[0]); // 렌더 타겟 설정

    ID3D11ShaderResourceView* BoundingBoxSRV = Renderer.GetStructuredBufferShaderResourceView(TEXT("BoundingBox"));
    Graphics.DeviceContext->VSSetShaderResources(2, 1, &BoundingBoxSRV);

    ID3D11ShaderResourceView* ConeSRV = Renderer.GetStructuredBufferShaderResourceView(TEXT("Cone"));
    Graphics.DeviceContext->VSSetShaderResources(3, 1, &ConeSRV);

    ID3D11ShaderResourceView* OBBSRV = Renderer.GetStructuredBufferShaderResourceView(TEXT("OBB"));
    Graphics.DeviceContext->VSSetShaderResources(4, 1, &OBBSRV);
}

void LineBatchRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    UPrimitiveBatch& PrimitveBatch = UPrimitiveBatch::GetInstance();
    
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);

    // 쉐이더 내에서 한 번만 Update되어야하는 CB
    const FMatrix Model = FMatrix::Identity;

    FMVPConstant MVPConstant;
    MVPConstant.M = Model;
    if (curEditorViewportClient != nullptr)
    {
        MVPConstant.VP = curEditorViewportClient->GetViewMatrix() * curEditorViewportClient->GetProjectionMatrix();
    }
    
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FMVPConstant")), &MVPConstant);

    const FGridParametersData GridParameters = PrimitveBatch.GetGridParameters();
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FGridParametersData")), &GridParameters);
    
    UpdateBatchResources();

    FPrimitiveCounts PrimitiveCounts;
    //PrimitiveCounts.ConeCount = PrimitveBatch.GetCones().Num();
    PrimitiveCounts.BoundingBoxCount = PrimitveBatch.GetBoundingBoxes().Num();
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FPrimitiveCounts")), &PrimitiveCounts);

    const std::shared_ptr<FVIBuffers> VIBuffer = Renderer.GetVIBuffer(TEXT("Line"));
    VIBuffer->Bind(Graphics.DeviceContext);

    const uint32 vertexCountPerInstance = 2;
    const uint32 instanceCount = GridParameters.GridCount + 3 + (PrimitveBatch.GetBoundingBoxes().Num() * 12) + (PrimitveBatch.GetCones().Num() * (2 * PrimitveBatch.GetConeSegmentCount()) + (12 * PrimitveBatch.GetOrientedBoundingBoxes().Num()));
    Graphics.DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);

    PrimitveBatch.ClearBatchPrimitives();
}

void LineBatchRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    // for (const USceneComponent* iter : TObjectRange<USceneComponent>())
    // {
    //     if (ULightComponent* pLightComp = Cast<ULightComponent>(iter))
    //     {
    //         FMatrix Model = JungleMath::CreateModelMatrix(pLightComp->GetWorldLocation(), pLightComp->GetWorldRotation(), {1, 1, 1});
    //         //UPrimitiveBatch::GetInstance().AddCone(pLightComp->GetWorldLocation(), pLightComp->GetAttenuationRadius(), 15, 140, pLightComp->GetLightColor(), Model);
    //         UPrimitiveBatch::GetInstance().AddOBB(pLightComp->GetBoundingBox(), pLightComp->GetWorldLocation(), Model);
    //     }
    // }
}

void LineBatchRenderPass::UpdateBatchResources() const
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    UPrimitiveBatch& PrimitveBatch = UPrimitiveBatch::GetInstance();
    
    {
        if (PrimitveBatch.GetBoundingBoxes().Num() > PrimitveBatch.GetAllocatedBoundingBoxCapacity())
        {
            PrimitveBatch.SetAllocatedBoundingBoxCapacity(PrimitveBatch.GetBoundingBoxes().Num());

            ID3D11Buffer* SB = nullptr;
            ID3D11ShaderResourceView* SBSRV = nullptr;
            SB = Renderer.CreateStructuredBuffer<FBoundingBox>(static_cast<uint32>(PrimitveBatch.GetAllocatedBoundingBoxCapacity()));
            SBSRV = Renderer.CreateBufferSRV(SB, static_cast<uint32>(PrimitveBatch.GetAllocatedBoundingBoxCapacity()));

            Renderer.AddOrSetStructuredBuffer(TEXT("BoundingBox"), SB);
            Renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("BoundingBox"), SBSRV);
        }

        ID3D11Buffer* SB = Renderer.GetStructuredBuffer(TEXT("BoundingBox"));
        ID3D11ShaderResourceView* SBSRV = Renderer.GetStructuredBufferShaderResourceView(TEXT("BoundingBox"));
        if (SB != nullptr && SBSRV != nullptr)
        {
            Renderer.UpdateStructuredBuffer(SB, PrimitveBatch.GetBoundingBoxes());
        }
    }
    
    //{
    //    if (UPrimitiveBatch::GetInstance().GetCones().Num() > UPrimitiveBatch::GetInstance().GetAllocatedConeCapacity())
    //    {
    //        UPrimitiveBatch::GetInstance().SetAllocatedConeCapacity(UPrimitiveBatch::GetInstance().GetCones().Num());

    //        ID3D11Buffer* SB = nullptr;
    //        ID3D11ShaderResourceView* SBSRV = nullptr;
    //        SB = Renderer.CreateStructuredBuffer<FBoundingBox>(static_cast<uint32>(UPrimitiveBatch::GetInstance().GetAllocatedConeCapacity()));
    //        SBSRV = Renderer.CreateBufferSRV(SB, static_cast<uint32>(UPrimitiveBatch::GetInstance().GetAllocatedConeCapacity()));

    //        Renderer.AddOrSetStructuredBuffer(TEXT("Cone"), SB);
    //        Renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("Cone"), SBSRV);
    //    }

    //    ID3D11Buffer* SB = Renderer.GetStructuredBuffer(TEXT("Cone"));
    //    ID3D11ShaderResourceView* SBSRV = Renderer.GetStructuredBufferShaderResourceView(TEXT("Cone"));
    //    if (SB != nullptr && SBSRV != nullptr)
    //    {
    //        Renderer.UpdateStructuredBuffer(SB, UPrimitiveBatch::GetInstance().GetCones());
    //    }
    //}
    
    {
        if (PrimitveBatch.GetOrientedBoundingBoxes().Num() > PrimitveBatch.GetAllocatedOBBCapacity())
        {
            PrimitveBatch.SetAllocatedOBBCapacity(PrimitveBatch.GetOrientedBoundingBoxes().Num());

            ID3D11Buffer* SB = nullptr;
            ID3D11ShaderResourceView* SBSRV = nullptr;
            SB = Renderer.CreateStructuredBuffer<FOBB>(static_cast<uint32>(PrimitveBatch.GetAllocatedOBBCapacity()));
            SBSRV = Renderer.CreateBufferSRV(SB, static_cast<uint32>(PrimitveBatch.GetAllocatedOBBCapacity()));

            Renderer.AddOrSetStructuredBuffer(TEXT("OBB"), SB);
            Renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("OBB"), SBSRV);
        }

        ID3D11Buffer* SB = Renderer.GetStructuredBuffer(TEXT("OBB"));
        ID3D11ShaderResourceView* SBSRV = Renderer.GetStructuredBufferShaderResourceView(TEXT("OBB"));
        if (SB != nullptr && SBSRV != nullptr)
        {
            Renderer.UpdateStructuredBuffer(SB, PrimitveBatch.GetOrientedBoundingBoxes());
        }
    }
}
