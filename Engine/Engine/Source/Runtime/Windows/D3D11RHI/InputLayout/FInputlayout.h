#pragma once
#include <d3d11.h>
#include <wrl/client.h>

#include "HAL/PlatformType.h"

class FInputlayout
{
public:
    FInputlayout();
    ~FInputlayout();

    void CreateInputLayout(uint32 vertexCount, D3D11_INPUT_ELEMENT_DESC* layout, const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength);
    void Bind();

private:
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_InputLayout;
};
