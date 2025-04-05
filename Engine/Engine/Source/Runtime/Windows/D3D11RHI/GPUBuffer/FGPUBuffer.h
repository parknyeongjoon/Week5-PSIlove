#pragma once
#include <d3d11.h>

class FGPUBuffer
{
public:
    ID3D11Buffer* buffer = nullptr;
    D3D11_BUFFER_DESC desc = {};
};
