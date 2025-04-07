#pragma once
#include "Container/String.h"

class FViewportClient;

class IRenderPass
{
public:
    IRenderPass(const FString& InShaderName)
        :  ShaderName(InShaderName) {}
    
    virtual void Prepare(FViewportClient* viewport) = 0;
    virtual void Execute(FViewportClient* viewport) = 0;
    
    virtual ~IRenderPass() {}
protected:
    // 렌더패스에서 사용할 리소스들
    FString ShaderName;
};