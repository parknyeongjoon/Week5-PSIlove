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

void FontRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
}

void FontRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;
    
    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
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
        Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FSubUVConstant")), &SubUVConstant);

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
        Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FConstants")), &Constant);

        const std::shared_ptr<FVIBuffers> currentVIBuffer = Renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(Graphics.DeviceContext);

        Graphics.DeviceContext->PSSetShaderResources(0,1, &item->Texture->TextureSRV);
        Graphics.DeviceContext->OMSetDepthStencilState(Renderer.GetDepthStencilState(EDepthStencilState::DepthNone), 0);

        Graphics.DeviceContext->Draw(currentVIBuffer->GetNumVertices(), 0);
    }
}

void FontRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    TextComponents.Empty();
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
        if (UTextRenderComponent* TextRenderComp = Cast<UTextRenderComponent>(iter))
        {
            TextComponents.Add(TextRenderComp);
        }
    }
}
