#include "DepthSceneRenderPass.h"

#include "EngineLoop.h"
#include "ViewportClient.h"

extern FEngineLoop GEngineLoop;

void DepthSceneRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewportClient)
{
    BaseRenderPass::Prepare(InViewportClient);
    
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    ID3D11RenderTargetView* writeRTV = Graphics.GetWriteRTV();
    Graphics.DeviceContext->OMSetRenderTargets(1, &writeRTV, nullptr);
    Graphics.DeviceContext->PSSetShaderResources(0, 1, &Graphics.DepthStencilSRV);
    
    Graphics.DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    
    Graphics.DeviceContext->RSSetState(Renderer.GetRasterizerState(Renderer.GetCurrentRasterizerState())); //레스터 라이저 상태 설정
    
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
}

void DepthSceneRenderPass::Execute(const std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    
    FUVBuffer uvBuffer;

    D3D11_VIEWPORT* d3dViewport = InViewportClient->GetD3DViewport();
    uvBuffer.UOffset = d3dViewport->TopLeftX / Graphics.screenWidth;
    uvBuffer.VOffset = d3dViewport->TopLeftY / Graphics.screenHeight;
    uvBuffer.UTiles = d3dViewport->Width / Graphics.screenWidth;
    uvBuffer.VTiles = d3dViewport->Height / Graphics.screenHeight;
    Renderer.UpdateConstantBuffer(Renderer.GetConstantBuffer(TEXT("FUVBuffer")), &uvBuffer);
    
    Graphics.DeviceContext->IASetInputLayout(nullptr); // 입력 레이아웃 불필요
    Graphics.DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics.DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, 0);
    
    Graphics.DeviceContext->Draw(6, 0); // 4개의 정점으로 화면 전체 사각형 그리기

    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Graphics.DeviceContext->PSSetShaderResources(0, 1, &nullSRV);

    // // Sampler 해제
    // ID3D11SamplerState* nullSamplers = nullptr;
    // Graphics.DeviceContext->PSSetSamplers(0, 1, &nullSamplers);
}

void DepthSceneRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
}
