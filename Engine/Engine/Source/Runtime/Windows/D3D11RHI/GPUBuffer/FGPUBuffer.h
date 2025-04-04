#pragma once
#include <d3d11.h>
#include <wrl.h>

class FGPUBuffer
{
public:
    
    FGPUBuffer() = default;
    virtual ~FGPUBuffer() = default;

protected:
    Microsoft::WRL::ComPtr<ID3D11Buffer> Buffer = nullptr;
    D3D11_BUFFER_DESC Desc = {};
};
