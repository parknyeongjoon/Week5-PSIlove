#include "FShaderProgram.h"

#include "EngineLoop.h"

extern FEngineLoop GEngineLoop;

void FShaderProgram::Bind(ID3D11DeviceContext* context) const
{
    FRenderer& Renderer = GEngineLoop.renderer;
    
    ID3D11VertexShader* VertexShader = Renderer.GetVertexShader(VSName);
    ID3D11PixelShader* PixelShader = Renderer.GetPixelShader(PSName);

    if (VertexShader)
    {
        context->VSSetShader(VertexShader, nullptr, 0);
    }
    else
    {
        context->VSSetShader(nullptr, nullptr, 0);
    }

    if (PixelShader)
    {
        context->PSSetShader(PixelShader, nullptr, 0);
    }
    else
    {
        context->PSSetShader(nullptr, nullptr, 0);
    }

    if (InputLayout)
    {
        context->IASetInputLayout(InputLayout);
    }
    else
    {
        context->IASetInputLayout(nullptr);
    }
}