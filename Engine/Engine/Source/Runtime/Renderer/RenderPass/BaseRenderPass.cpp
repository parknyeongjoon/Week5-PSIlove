#include "BaseRenderPass.h"

#include "EngineLoop.h"
#include "ViewportClient.h"
extern FEngineLoop GEngineLoop;

void BaseRenderPass::Prepare(const std::shared_ptr<FViewportClient> InViewportClient)
{
    GEngineLoop.renderer.PrepareShader(ShaderName);
}
