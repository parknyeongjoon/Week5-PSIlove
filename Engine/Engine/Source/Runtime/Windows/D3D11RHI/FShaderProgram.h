#pragma once
#include "Define.h"

class FShaderProgram
{
public:
    FShaderProgram(/*ID3D11VertexShader* InVertexShader, ID3D11PixelShader* InPixelShader, */FString InVSName, FString InPSName, ID3D11InputLayout* InInputLayout)
        : /*VertexShader(InVertexShader), PixelShader(InPixelShader),*/ VSName(InVSName), PSName(InPSName), InputLayout(InInputLayout)
    {}
    // 생성자 (필요한 경우 초기화)
    FShaderProgram() = default;

    // 셰이더 및 입력 레이아웃 바인딩 함수
    void Bind(ID3D11DeviceContext* context) const;

    void Release()
    {
        // if (VertexShader)
        // {
        //     VertexShader->Release();
        //     VertexShader = nullptr;
        // }
        //
        // if (PixelShader)
        // {
        //     PixelShader->Release();
        //     PixelShader = nullptr;
        // }

        if (InputLayout)
        {
            InputLayout->Release();
            InputLayout = nullptr;
        }
    }

    //void SetVertexShader(ID3D11VertexShader* InVertexShader) { VertexShader = InVertexShader;}
    //void SetPixelShader(ID3D11PixelShader* InPixelShader) { PixelShader = InPixelShader;}
    void SetInputLayout(ID3D11InputLayout* InInputLayout) { InputLayout = InInputLayout;}
    
    void SetVertexShaderName(const FString& InName) { VSName = InName;}
    void SetPixelShaderName(const FString& InName) { PSName = InName;}
    
    //ID3D11VertexShader* GetVertexShader() const { return VertexShader; }
    //ID3D11PixelShader*  GetPixelShader() const { return PixelShader; }
    ID3D11InputLayout*  GetInputLayout() const { return InputLayout; }

    FString GetVertexShaderName() { return VSName;}
    FString GetPixelShaderName() { return PSName;}

private:
    FString VSName;
    FString PSName;
    
    //ID3D11VertexShader* VertexShader;
    //ID3D11PixelShader*  PixelShader;
    ID3D11InputLayout*  InputLayout;
};
