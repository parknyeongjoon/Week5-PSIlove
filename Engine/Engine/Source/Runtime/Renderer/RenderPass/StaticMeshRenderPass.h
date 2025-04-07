#pragma once
#include "IRenderPass.h"
#include "Container/Array.h"

class UPrimitiveComponent;

class StaticMeshRenderPass: public IRenderPass
{
public:
    StaticMeshRenderPass(const FString& InShaderName)
        : IRenderPass(InShaderName) {}
    
    virtual void Prepare(FViewportClient* viewport) override;
    virtual void Execute(FViewportClient* viewport) override;

    void AddPrimitive(UPrimitiveComponent* Primitive);
private:
    TArray<UPrimitiveComponent*> Primitives;
};