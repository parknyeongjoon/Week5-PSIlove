#include "FShaderProgram.h"

void FShaderProgram::Release()
{
    VertexShader->Release();
    PixelShader->Release();
    InputLayout->Release();
}
