#pragma once
#include "BaseRenderPass.h"

class LineBatchRenderPass : public BaseRenderPass
{
public:
    explicit LineBatchRenderPass(const FString& InShaderName);

    virtual void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
private:
    void UpdateBatchResources() const;
};
