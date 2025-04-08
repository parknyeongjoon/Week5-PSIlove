#include "FontRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "Components/TextBillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

void FontRenderPass::Prepare(const std::shared_ptr<FViewportClient> viewport)
{
    BaseRenderPass::Prepare(viewport);
    GEngineLoop.graphicDevice.DeviceContext->OMSetDepthStencilState(GEngineLoop.renderer .GetDepthStencilState(EDepthStencilState::DepthNone), 0);
}

void FontRenderPass::Execute(const std::shared_ptr<FViewportClient> viewport)
{
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;
    
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(viewport);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }
    
    for (const auto item : TextComponents)
    {
        FSubUVConstant SubUVConstant;
        SubUVConstant.indexU = item->finalIndexU;
        SubUVConstant.indexV = item->finalIndexV;
        GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FSubUVConstant")), &SubUVConstant);

        FMatrix Model;
        if (UTextBillboardComponent* TextBillboardComponent = Cast<UTextBillboardComponent>(item))
        {
            Model = TextBillboardComponent->CreateBillboardMatrix();
        }
        else if (UTextRenderComponent* Text = Cast<UTextRenderComponent>(item))
        {
            Model = JungleMath::CreateModelMatrix(Text->GetWorldLocation(), Text->GetWorldRotation(), Text->GetWorldScale());
        }

        FMatrix VP = View * Proj;
            
        FConstants Constant;
        Constant.MVP = Model * VP;
        GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FConstants")), &Constant);

        const std::shared_ptr<FVIBuffers> currentVIBuffer =  GEngineLoop.renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(GEngineLoop.graphicDevice.DeviceContext);

        GEngineLoop.graphicDevice.DeviceContext->OMSetDepthStencilState(GEngineLoop.renderer.GetDepthStencilState(EDepthStencilState::DepthNone), 0);
        GEngineLoop.graphicDevice.DeviceContext->PSSetShaderResources(0,1, &item->Texture->TextureSRV);
        GEngineLoop.graphicDevice.DeviceContext->Draw(currentVIBuffer->GetNumVertices(), 0);
    }
}

void FontRenderPass::AddRenderObjectsToRenderPass(const ULevel* Level)
{
    TextComponents.Empty();
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
        if (UTextRenderComponent* TextRenderComp = Cast<UTextRenderComponent>(iter))
        {
            TextComponents.Add(TextRenderComp);
        }
    }
}
