#pragma once
#include "BaseRenderPass.h"

class LineBatchRenderPass : public BaseRenderPass
{
public:
    explicit LineBatchRenderPass(const FString& InShaderName);

    virtual void Prepare(std::shared_ptr<FViewportClient> viewport) override;
    virtual void Execute(std::shared_ptr<FViewportClient> viewport) override;

    void AddRenderObjectsToRenderPass(const ULevel* Level) override;
private:
    void UpdateBatchResources() const;
};
