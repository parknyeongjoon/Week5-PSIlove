#pragma once
#include "HAL/PlatformType.h"

#define _TCHAR_DEFINED
#include <wrl.h>
#include <d3d11.h>

class FConstantBuffer
{
    FConstantBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, const uint32 InSize)
        : Buffer(InBuffer), Size(InSize) {}
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer = nullptr;
    
    uint32 Slot = 0;
    uint32 Size = 0;
};
