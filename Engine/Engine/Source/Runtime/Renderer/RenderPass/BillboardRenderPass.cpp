#include "BillboardRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "Components/BillboardComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "GameFramework/Actor.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

void BillboardRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
    
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    Graphics.DeviceContext->RSSetState(Renderer.GetRasterizerState(Renderer.GetCurrentRasterizerState()));
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
}

void BillboardRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;

    const std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }
    
     for (UBillboardComponent* item : BillboardComponents)
     {
         FMatrix Model = item->CreateBillboardMatrix();
         FMatrix VP = View * Proj;
            
         FConstants Constant;
         Constant.MVP = Model * VP;
         Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FConstants")), &Constant);
         
         FSubUVConstant SubUVConstant;
         SubUVConstant.indexU = item->finalIndexU;
         SubUVConstant.indexV = item->finalIndexV;
         Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FSubUVConstant")), &SubUVConstant);

         const std::shared_ptr<FVIBuffers> currentVIBuffer = Renderer.GetVIBuffer(item->VIBufferName);
         currentVIBuffer->Bind(Graphics.DeviceContext);
         Graphics.DeviceContext->PSSetShaderResources(0, 1, &(item->Texture->TextureSRV));

         // if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(item))
         // {
         //     GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(currentVIBuffer->GetNumVertices(), 0, 0);
         // }
         // else
         {
             Graphics.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0, 0);
         }

     }
}

void BillboardRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    BillboardComponents.Empty();
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
        if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
        {
            BillboardComponents.Add(pBillboardComp);
        }
    }
}
