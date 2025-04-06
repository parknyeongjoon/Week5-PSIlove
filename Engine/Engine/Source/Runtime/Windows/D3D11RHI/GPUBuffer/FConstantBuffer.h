#pragma once
#include "HAL/PlatformType.h"

class D3D11Buffer;

class FConstantBuffer
{
    FConstantBuffer(D3D11Buffer* InBuffer, const uint32 InSize)
        : Buffer(InBuffer), Size(InSize) {}
private:
    D3D11Buffer* Buffer = nullptr;
    
    uint32 Slot = 0;
    uint32 Size = 0;
};
