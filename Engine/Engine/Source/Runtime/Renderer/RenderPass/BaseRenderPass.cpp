#include "BaseRenderPass.h"

#include "EngineLoop.h"
#include "ViewportClient.h"
extern FEngineLoop GEngineLoop;

void BaseRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    GEngineLoop.renderer.PrepareShader(ShaderName);

    GEngineLoop.graphicDevice.DeviceContext->RSSetState(GEngineLoop.renderer.GetRasterizerState(ERasterizerState::SolidBack));
    ID3D11SamplerState* linearSampler = GEngineLoop.renderer.GetSamplerState(ESamplerType::Linear);
    GEngineLoop.graphicDevice.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
}
