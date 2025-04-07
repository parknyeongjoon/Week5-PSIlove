#pragma once
#include "Define.h"

class FShaderProgram
{
public:
    FShaderProgram(const Microsoft::WRL::ComPtr<ID3D11VertexShader>& InVertexShader, const Microsoft::WRL::ComPtr<ID3D11PixelShader>& InPixelShader, const Microsoft::WRL::ComPtr<ID3D11InputLayout>& InInputLayout, const uint32 InStride)
        : VertexShader(InVertexShader), PixelShader(InPixelShader), InputLayout(InInputLayout), Stride(InStride)
    {}
    // 생성자 (필요한 경우 초기화)
    FShaderProgram() = default;

    // 셰이더 및 입력 레이아웃 바인딩 함수
    void Bind(ID3D11DeviceContext* context) const
    {
        context->IASetInputLayout(InputLayout.Get());
        context->VSSetShader(VertexShader.Get(), nullptr, 0);
        context->PSSetShader(PixelShader.Get(), nullptr, 0);
    }

    void SetVertexShader(const Microsoft::WRL::ComPtr<ID3D11VertexShader>& InVertexShader) { VertexShader = InVertexShader;}
    void SetPixelShader(const Microsoft::WRL::ComPtr<ID3D11PixelShader>& InPixelShader) { PixelShader = InPixelShader;}
    void SetInputLayout(const Microsoft::WRL::ComPtr<ID3D11InputLayout>& InInputLayout) { InputLayout = InInputLayout;}

    Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader() const { return VertexShader; }
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  GetPixelShader() const { return PixelShader; }
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  GetInputLayout() const { return InputLayout; }

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>  PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout>  InputLayout;
    uint32 Stride;
};
