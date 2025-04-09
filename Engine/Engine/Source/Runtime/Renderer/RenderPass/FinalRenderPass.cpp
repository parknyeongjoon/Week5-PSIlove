#include "FinalRenderPass.h"

#include "Define.h"
#include "EngineLoop.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

FinalRenderPass::FinalRenderPass(const FString& InName)
    : BaseRenderPass(InName)
{
}

void FinalRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewport)
{
    BaseRenderPass::Prepare(InViewport);
    
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    Graphics.SwapRTV();
    Graphics.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    Graphics.DeviceContext->RSSetState(Renderer.GetRasterizerState(ERasterizerState::SolidBack)); //레스터 라이저 상태 설정
    Graphics.DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics.DeviceContext->OMSetRenderTargets(1, &Graphics.FrameBufferRTV, nullptr); // 렌더 타겟 설정(백버퍼를 가르킴)
}

void FinalRenderPass::Execute(std::shared_ptr<FViewportClient> InViewportClient)
{
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    FRenderer& Renderer = GEngineLoop.renderer;

    std::shared_ptr<FEditorViewportClient> activeViewport = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    
    FUVBuffer uvBuffer;

    D3D11_VIEWPORT* d3dViewport = InViewportClient->GetD3DViewport();
    uvBuffer.UOffset = d3dViewport->TopLeftX / Graphics.screenWidth;
    uvBuffer.VOffset = d3dViewport->TopLeftY / Graphics.screenHeight;
    uvBuffer.UTiles = d3dViewport->Width / Graphics.screenWidth;
    uvBuffer.VTiles = d3dViewport->Height / Graphics.screenHeight;
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FUVBuffer")), &uvBuffer);
    
    // SceneColor + Depth SRV 바인딩
    // SceneColor + Depth SRV 바인딩
    ID3D11ShaderResourceView* SRVs[2] = { Graphics.GetReadSRV(), Graphics.DepthStencilSRV};
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

void FinalRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
}