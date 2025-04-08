#include "BillboardRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "Components/BillboardComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "GameFramework/Actor.h"
#include "UnrealEd/EditorViewportClient.h"

class UParticleSubUVComp;
extern FEngineLoop GEngineLoop;

void BillboardRenderPass::Prepare(const std::shared_ptr<FViewportClient> viewport)
{
    BaseRenderPass::Prepare(viewport);
}

void BillboardRenderPass::Execute(const std::shared_ptr<FViewportClient> viewport)
{
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;
    
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(viewport);
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
         GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FConstants")), &Constant);
         
         FSubUVConstant SubUVConstant;
         SubUVConstant.indexU = item->finalIndexU;
         SubUVConstant.indexV = item->finalIndexV;
         GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FSubUVConstant")), &SubUVConstant);

         const std::shared_ptr<FVIBuffers> currentVIBuffer = GEngineLoop.renderer.GetVIBuffer(item->VIBufferName);
         currentVIBuffer->Bind(GEngineLoop.graphicDevice.DeviceContext);
         GEngineLoop.graphicDevice.DeviceContext->PSSetShaderResources(0, 1, &(item->Texture->TextureSRV));

         // if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(item))
         // {
         //     GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(currentVIBuffer->GetNumVertices(), 0, 0);
         // }
         // else
         {
             GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0, 0);
         }

     }
}

void BillboardRenderPass::AddRenderObjectsToRenderPass(const ULevel* Level)
{
    BillboardComponents.Empty();
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
        if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
        {
            BillboardComponents.Add(pBillboardComp);
        }
    }
}
