#include "LightingRenderPass.h"

#include "Level.h"
#include "ViewportClient.h"
#include "Components/LightComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

void LightingRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    LightComponents.Empty();
    TArray<USceneComponent*> Ss;
     for (const auto& A : InLevel->GetActors())
     {
         Ss.Add(A->GetRootComponent());
         TArray<UActorComponent*> components;
         components = A->GetComponents();
         
         for (const auto& comp : components)
         {
             if (ULightComponent* pLightComp = Cast<ULightComponent>(comp))
                 LightComponents.Add(pLightComp);
         }
     }
}

void LightingRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
    FGraphicsDevice& GraphicDevice = GEngineLoop.graphicDevice;

    auto* RenderTarget = GraphicDevice.GetWriteRTV();
    GraphicDevice.DeviceContext->OMSetDepthStencilState(nullptr, 0);
    GraphicDevice.DeviceContext->OMSetRenderTargets(1, &RenderTarget, nullptr);
    GraphicDevice.DeviceContext->PSSetShaderResources(0, 4, GraphicDevice.GBufferSRVs);
}

void LightingRenderPass::Execute(std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    UVBuffer uvBuffer;
    uvBuffer.UOffset = InViewportClient->GetD3DViewport()->TopLeftX / Graphics.screenWidth;
    uvBuffer.VOffset = InViewportClient->GetD3DViewport()->TopLeftY / Graphics.screenHeight;
    uvBuffer.UTiles = InViewportClient->GetD3DViewport()->Width / Graphics.screenWidth;
    uvBuffer.VTiles = InViewportClient->GetD3DViewport()->Height / Graphics.screenHeight;
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("UVBuffer")), &uvBuffer);

    LightBuffer lightBuffer;
    lightBuffer.EyePosition = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewTransformPerspective.ViewLocation;
    lightBuffer.LightCount = LightComponents.Num();
    for (int i = 0; i < LightComponents.Num(); i++)
    {
        lightBuffer.Lights[i].Intensity = LightComponents[i]->GetIntensity();
        lightBuffer.Lights[i].Position = LightComponents[i]->GetOwner()->GetActorLocation();
        lightBuffer.Lights[i].AmbientFactor = 0.0f;
        lightBuffer.Lights[i].LightColor = LightComponents[i]->GetLightColor();
        lightBuffer.Lights[i].LightDirection = FVector(-1.f, -1.f, -1.f);
        lightBuffer.Lights[i].AttenuationRadius = LightComponents[i]->GetAttenuationRadius();
    }
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("LightBuffer")), &lightBuffer);

    Graphics.DeviceContext->Draw(6, 0); // 4개의 정점으로 화면 전체 사각형 그리기
    
    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[4] = { nullptr, nullptr, nullptr, nullptr };
    Graphics.DeviceContext->PSSetShaderResources(0, 4, nullSRV);

    // Sampler 해제
    ID3D11SamplerState* nullSamplers[1] = { nullptr };
    Graphics.DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}
