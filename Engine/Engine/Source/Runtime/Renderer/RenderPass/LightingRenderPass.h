#pragma once
#include "BaseRenderPass.h"
#include "Container/Array.h"

class ULightComponent;

class LightingRenderPass : public BaseRenderPass
{
public:
    LightingRenderPass(const FString& InShaderName)
        : BaseRenderPass(InShaderName) {}
    
    virtual void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;
private:
    TArray<ULightComponent*> LightComponents;
};
