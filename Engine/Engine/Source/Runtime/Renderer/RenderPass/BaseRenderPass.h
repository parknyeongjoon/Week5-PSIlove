#pragma once
#include "Container/String.h"

class ULevel;
class FViewportClient;

class BaseRenderPass
{
public:
    BaseRenderPass(const FString& InShaderName)
        :  ShaderName(InShaderName) {}

    virtual void AddRenderObjectsToRenderPass(const ULevel* InLevel) = 0;
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewportClient);
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) = 0;
    
    virtual ~BaseRenderPass() {}
protected:
    // 렌더패스에서 사용할 리소스들
    FString ShaderName;
};