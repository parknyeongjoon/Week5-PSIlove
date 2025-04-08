#pragma once
#include "BaseRenderPass.h"
#include "Container/Array.h"

class UBillboardComponent;

class BillboardRenderPass : public BaseRenderPass
{
public:
    BillboardRenderPass(const FString& InName)
        : BaseRenderPass(InName) {}
    virtual void Prepare(std::shared_ptr<FViewportClient> viewport) override;
    virtual void Execute(std::shared_ptr<FViewportClient> viewport) override;

    void AddRenderObjectsToRenderPass(const ULevel* Level) override;
    void AddBillboard(UBillboardComponent* billboard) { BillboardComponents.Add(billboard); }
private:
    TArray<UBillboardComponent*> BillboardComponents;
};
