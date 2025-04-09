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
    
    GraphicDevice.DeviceContext->RSSetState(Renderer.GetRasterizerState(Renderer.GetCurrentRasterizerState()));
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    GraphicDevice.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
    
    GraphicDevice.SwapRTV();
    ID3D11RenderTargetView* RenderTarget = GraphicDevice.GetWriteRTV();
    GraphicDevice.DeviceContext->OMSetDepthStencilState(nullptr, 0);
    GraphicDevice.DeviceContext->OMSetRenderTargets(1, &RenderTarget, nullptr); // 렌더 타겟 설정
}

void FogRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewport)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    std::shared_ptr<FEditorViewportClient> activeViewport = std::dynamic_pointer_cast<FEditorViewportClient>(InViewport);
    UpdateFogConstant(activeViewport);

    UpdateFogQuadVertexBufferUpdate(activeViewport);

    // SceneColor + Depth SRV 바인딩
    ID3D11ShaderResourceView* SRVs[] = { Graphics.GetReadSRV(), Graphics.pingpongDepthSRV[0] };
    Graphics.DeviceContext->PSSetShaderResources(0, 2, SRVs);

    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(0, 1, &linearSampler);

    const std::shared_ptr<FVIBuffers> currentVIBuffer = Renderer.GetVIBuffer(FogComponent->VIBufferName);
    currentVIBuffer->Bind(Graphics.DeviceContext);

    // 풀스크린 쿼드 그리기
    Graphics.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0, 0);

    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    Graphics.DeviceContext->PSSetShaderResources(0, 2, nullSRV);

    // // Sampler 해제
    // ID3D11SamplerState* nullSamplers[1] = { nullptr };
    // Graphics.DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}

void FogRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
    FogComponent = InLevel->GetFog()->GetComponentByClass<UHeightFogComponent>();
}

void FogRenderPass::UpdateFogConstant(const std::shared_ptr<FEditorViewportClient>& InActiveViewport) const
{
    FRenderer& Renderer = GEngineLoop.renderer;

    FFogConstants fogConstants;
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
    
    Renderer.UpdateConstnatBuffer(Renderer.GetConstantBuffer(TEXT("FFogConstants")), &fogConstants);
}

void FogRenderPass::UpdateFogQuadVertexBufferUpdate(const std::shared_ptr<FEditorViewportClient>& InActiveViewport) const
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    const float screenWidth = static_cast<float>(Graphics.screenWidth);
    const float screenHeight = static_cast<float>(Graphics.screenHeight);

    D3D11_VIEWPORT* activeD3DViewport = InActiveViewport->GetD3DViewport();
    const float uvMinX = activeD3DViewport->TopLeftX / screenWidth;
    const float uvMinY = activeD3DViewport->TopLeftY / screenHeight;
    const float uvMaxX = (activeD3DViewport->TopLeftX + activeD3DViewport->Width) / screenWidth;
    const float uvMaxY = (activeD3DViewport->TopLeftY + activeD3DViewport->Height) / screenHeight;

    FScreenVertex vertices[4] = {
        { FVector4(-1.0f, 1.0f, 0.0f, 1.0f), uvMinX, uvMinY }, // top-left
        { FVector4(1.0f, 1.0f, 0.0f, 1.0f), uvMaxX, uvMinY }, // top-right
        { FVector4(1.0f, -1.0f, 0.0f, 1.0f), uvMaxX, uvMaxY }, // bottom-right
        { FVector4(-1.0f, -1.0f, 0.0f, 1.0f), uvMinX, uvMaxY }  // bottom-left
    };

    ID3D11Buffer* fogVertexBuffer = Renderer.GetVIBuffer(FogComponent->VIBufferName)->GetVertexBuffer();
    Renderer.UpdateVertexBuffer(fogVertexBuffer, &vertices, 4);
}
