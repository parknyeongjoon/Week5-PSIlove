#include "FogRenderPass.h"

#include "EngineLoop.h"
#include "Level.h"
#include "Actors/Fog.h"
#include "Components/HeightFogComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "UnrealEd/EditorViewportClient.h"


extern FEngineLoop GEngineLoop;

void FogRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewport)
{
    BaseRenderPass::Prepare(InViewport);

    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& GraphicDevice = GEngineLoop.graphicDevice;

    GraphicDevice.SwapRTV();
    ID3D11RenderTargetView* RenderTarget = GraphicDevice.GetWriteRTV();
    GraphicDevice.DeviceContext->OMSetDepthStencilState(nullptr, 0);
    GraphicDevice.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    GraphicDevice.DeviceContext->RSSetState(Renderer.GetRasterizerState(ERasterizerState::SolidBack));
    GraphicDevice.DeviceContext->OMSetRenderTargets(1, &RenderTarget, nullptr); // 렌더 타겟 설정
}

void FogRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    std::shared_ptr<FEditorViewportClient> activeViewport = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    UpdateFogConstant(activeViewport);

    FUVBuffer uvBuffer;

    D3D11_VIEWPORT* d3dViewport = InViewportClient->GetD3DViewport();
    uvBuffer.UOffset = d3dViewport->TopLeftX / Graphics.screenWidth;
    uvBuffer.VOffset = d3dViewport->TopLeftY / Graphics.screenHeight;
    uvBuffer.UTiles = d3dViewport->Width / Graphics.screenWidth;
    uvBuffer.VTiles = d3dViewport->Height / Graphics.screenHeight;
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FUVBuffer")), &uvBuffer);

    // SceneColor + Depth SRV 바인딩
    ID3D11ShaderResourceView* SRVs[] = { Graphics.GetReadSRV(), Graphics.DepthStencilSRV };
    Graphics.DeviceContext->PSSetShaderResources(0, 2, SRVs);

        
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);

    Graphics.DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics.DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);
    Graphics.DeviceContext->IASetInputLayout(nullptr);

    // 풀스크린 쿼드 그리기
    Graphics.DeviceContext->Draw(6, 0);

    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    Graphics.DeviceContext->PSSetShaderResources(0, 2, nullSRV);

    // // Sampler 해제
    // ID3D11SamplerState* nullSamplers[1] = { nullptr };
    // Graphics.DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}

void FogRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    TArray<USceneComponent*> Ss;
    for (const auto& A : InLevel->GetActors())
    {
        Ss.Add(A->GetRootComponent());
        TArray<UActorComponent*> components;
        components = A->GetComponents();
        for (const auto& comp : components)
        {
            if (UHeightFogComponent* pHeightFogComp = Cast<UHeightFogComponent>(comp))
            {
                FogComponent = pHeightFogComp;
                return;
            }
        }
    }
}

void FogRenderPass::UpdateFogConstant(const std::shared_ptr<FEditorViewportClient>& InActiveViewport) const
{
    FRenderer& Renderer = GEngineLoop.renderer;

    FogConstants fogConstants;
    fogConstants.FogDensity = FogComponent->FogDensity;
    fogConstants.FogHeightFalloff = FogComponent->FogHeightFalloff;
    fogConstants.StartDistance = FogComponent->StartDistance;
    fogConstants.FogCutoffDistance = FogComponent->FogCutoffDistance;
    fogConstants.FogMaxOpacity = FogComponent->FogMaxOpacity;
    fogConstants.FogInscatteringColor = FVector4(FogComponent->FogInscatteringColor.r, FogComponent->FogInscatteringColor.g, FogComponent->FogInscatteringColor.b, FogComponent->FogInscatteringColor.a);
    
    fogConstants.InvProjectionMatrix = FMatrix::Inverse(InActiveViewport->GetProjectionMatrix());
    fogConstants.InvViewMatrix = FMatrix::Inverse(InActiveViewport->GetViewMatrix());
    
    const FVector cameraPos = InActiveViewport->ViewTransformPerspective.GetLocation();
    fogConstants.CameraWorldPos = FVector4(cameraPos.x,cameraPos.y, cameraPos.z, 1);
    
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FogConstants")), &fogConstants);
}