#pragma once
#include "Container/Array.h"
#include "Container/String.h"

class UPrimitiveComponent;
class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11VertexShader;
class ID3D11PixelShader;
class ID3D11InputLayout;
class ID3D11RenderTargetView;
class ID3D11DepthStencilView;
class ID3D11Buffer;
class D3D11_VIEWPORT;

class IRenderPass
{
public:
    IRenderPass(const FString& InShaderName)
        :  ShaderName(InShaderName) {}
    
    virtual void Prepare(D3D11_VIEWPORT* viewport) = 0;
    virtual void Execute(D3D11_VIEWPORT* viewport) = 0;
    
    virtual ~IRenderPass() {}
protected:
    // 렌더패스에서 사용할 리소스들
    FString ShaderName;
};

class OpaqueRenderPass : public IRenderPass
{
public:
    OpaqueRenderPass(const FString& InShaderName)
        : IRenderPass(InShaderName) {}
    
    virtual void Prepare(D3D11_VIEWPORT* viewport) override;
    virtual void Execute(D3D11_VIEWPORT* viewport) override;

    void AddPrimitive(UPrimitiveComponent* Primitive);
private:
    TArray<UPrimitiveComponent*> Primitives;
};
