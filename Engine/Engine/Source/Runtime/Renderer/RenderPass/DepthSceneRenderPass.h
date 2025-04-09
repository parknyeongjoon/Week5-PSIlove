#pragma once
#include "BaseRenderPass.h"

class DepthSceneRenderPass : public BaseRenderPass
{
public:
    DepthSceneRenderPass(const FString& InName)
        : BaseRenderPass(InName) {}
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
    
};
