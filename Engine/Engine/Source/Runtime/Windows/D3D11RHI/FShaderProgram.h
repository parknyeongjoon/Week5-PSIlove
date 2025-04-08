#pragma once
#include "Define.h"

class FShaderProgram
{
public:
    FShaderProgram(ID3D11VertexShader* InVertexShader, ID3D11PixelShader* InPixelShader, ID3D11InputLayout* InInputLayout, const uint32 InStride)
        : VertexShader(InVertexShader), PixelShader(InPixelShader), InputLayout(InInputLayout), Stride(InStride)
    {}
    // 생성자 (필요한 경우 초기화)
    FShaderProgram() = default;

    // 셰이더 및 입력 레이아웃 바인딩 함수
    void Bind(ID3D11DeviceContext* context) const
    {
        context->IASetInputLayout(InputLayout);
        context->VSSetShader(VertexShader, nullptr, 0);
        context->PSSetShader(PixelShader, nullptr, 0);
    }

    void Release()
    {
        if (VertexShader)
        {
            VertexShader->Release();
            VertexShader = nullptr;
        }

        if (PixelShader)
        {
            PixelShader->Release();
            PixelShader = nullptr;
        }

        if (InputLayout)
        {
            InputLayout->Release();
            InputLayout = nullptr;
        }
    }

    void SetVertexShader(ID3D11VertexShader* InVertexShader) { VertexShader = InVertexShader;}
    void SetPixelShader(ID3D11PixelShader* InPixelShader) { PixelShader = InPixelShader;}
    void SetInputLayout(ID3D11InputLayout* InInputLayout) { InputLayout = InInputLayout;}

    ID3D11VertexShader* GetVertexShader() const { return VertexShader; }
    ID3D11PixelShader*  GetPixelShader() const { return PixelShader; }
    ID3D11InputLayout*  GetInputLayout() const { return InputLayout; }

private:
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader*  PixelShader;
    ID3D11InputLayout*  InputLayout;
    uint32 Stride;
};
