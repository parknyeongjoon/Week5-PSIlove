#pragma once
#include "Container/String.h"

class ULevel;
class FViewportClient;

class BaseRenderPass
{
public:
    BaseRenderPass(const FString& InShaderName)
        :  ShaderName(InShaderName) {}

    virtual void AddRenderObjectsToRenderPass(const ULevel* Level) = 0;
    virtual void Prepare(std::shared_ptr<FViewportClient> viewport);
    virtual void Execute(std::shared_ptr<FViewportClient> viewport) = 0;
    
    virtual ~BaseRenderPass() {}
protected:
    // 렌더패스에서 사용할 리소스들
    FString ShaderName;
};