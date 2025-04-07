#pragma once
#include "IRenderPass.h"

class UPrimitiveBatch;

class LineBatchRenderPass : public IRenderPass
{
public:
    
    explicit LineBatchRenderPass(const FString& InShaderName);

    virtual void Prepare(FViewportClient* viewport) override;
    virtual void Execute(FViewportClient* viewport) override;

    void SetPrimitveBatch(UPrimitiveBatch* batch) { PrimitiveBatch = batch; }
private:
    UPrimitiveBatch* PrimitiveBatch;
};
