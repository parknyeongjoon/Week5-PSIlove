#pragma once

#include "Define.h"
#include "Container/Array.h"

class UPrimitiveComponent;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
class ID3D11Device;
class ID3D11DeviceContext;
class ID3D11VertexShader;

class IRenderPass
{
public:
    // 렌더 패스 초기화 (셰이더 로딩, 렌더 타겟 생성 등)
    virtual void Setup(ID3D11Device* device) = 0;

    // 해당 패스의 드로우 콜 실행: 필요한 상태 설정 후 액터 혹은 메시 렌더링
    virtual void Execute(ID3D11DeviceContext* context) = 0;

    virtual ~IRenderPass() {}
};

class FowardRenderPass : public IRenderPass
{
public:
    ~FowardRenderPass()
    {
        VertexShader = nullptr;
        PixelShader = nullptr;
        InputLayout = nullptr;
        Renderables.Empty();
    }
    
    explicit FowardRenderPass(ID3D11VertexShader* InVertexShader, ID3D11PixelShader* InPixelShader, ID3D11InputLayout* InInputLayout);
    virtual void Setup(ID3D11Device* device);
    virtual void Execute(ID3D11DeviceContext* context);
private:
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;

    TArray<UPrimitiveComponent*> Renderables;
};