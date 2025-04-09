#pragma once
#include "BaseRenderPass.h"
#include "Container/Array.h"

struct FMatrix;
class UTextRenderComponent;

class FontRenderPass : public BaseRenderPass
{
public:
    FontRenderPass(const FString& InShaderName)
        : BaseRenderPass(InShaderName) {}
    
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
    void AddTextRenderComponent(UTextRenderComponent* InTextRenderComponent) { TextComponents.Add(InTextRenderComponent); }
private:
    TArray<UTextRenderComponent*> TextComponents;
};
