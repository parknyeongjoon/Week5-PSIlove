#pragma once
#include "BaseRenderPass.h"

class FEditorViewportClient;

class FinalRenderPass : public BaseRenderPass
{
public:
    FinalRenderPass(const FString& InName);
    
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewport) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
private:
    FString VIBufferName;
};
