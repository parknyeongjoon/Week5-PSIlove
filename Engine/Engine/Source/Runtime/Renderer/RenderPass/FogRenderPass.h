#pragma once
#include "BaseRenderPass.h"

class FEditorViewportClient;
class UHeightFogComponent;

class FogRenderPass : public BaseRenderPass
{
public:
    FogRenderPass(const FString& InName)
        : BaseRenderPass(InName) {}
    
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewport) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
private:
    void UpdateFogConstant(const std::shared_ptr<FEditorViewportClient>& InActiveViewport) const;
    UHeightFogComponent* FogComponent;
};

