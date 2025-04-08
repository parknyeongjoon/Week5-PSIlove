#include "BaseRenderPass.h"

#include "EngineLoop.h"
#include "ViewportClient.h"
extern FEngineLoop GEngineLoop;

void BaseRenderPass::Prepare(const std::shared_ptr<FViewportClient> viewport)
{
    GEngineLoop.graphicDevice.DeviceContext->OMSetRenderTargets(1, &GEngineLoop.graphicDevice.RTVs[0], GEngineLoop.graphicDevice.DepthStencilView);
    
    GEngineLoop.graphicDevice.DeviceContext->RSSetState(GEngineLoop.renderer.GetRasterizerState(GEngineLoop.renderer.GetCurrentRasterizerState())); //레스터 라이저 상태 설정
    GEngineLoop.graphicDevice.DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
    GEngineLoop.graphicDevice.DeviceContext->RSSetViewports(1, viewport->GetD3DViewport());

    GEngineLoop.renderer.PrepareShader(ShaderName);
}
