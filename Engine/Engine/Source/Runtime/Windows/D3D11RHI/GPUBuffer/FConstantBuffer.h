#pragma once
#include "HAL/PlatformType.h"
#include <d3d11.h>

class D3D11Buffer;

class FConstantBuffer
{
    FConstantBuffer();
public:
    D3D11Buffer* buffer = nullptr;
    D3D11_BUFFER_DESC desc = {};
    
    uint32 slot = 0;
};
