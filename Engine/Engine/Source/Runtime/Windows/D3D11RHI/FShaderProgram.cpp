#include "FShaderProgram.h"

void FShaderProgram::Release()
{
    VertexShader->Release();
    VertexShader = nullptr;
    PixelShader->Release();
    PixelShader = nullptr;
    InputLayout->Release();
    InputLayout = nullptr;
}
