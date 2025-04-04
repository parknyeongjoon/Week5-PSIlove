#include "FIndexBuffer.h"

#include <cassert>

#include "GraphicDevice.h"
#include "D3D11RHI/GraphicDevice.h"

bool FIndexBuffer::Create(FGraphicsDevice* InGraphicDevice, const TArray<uint32>& indices)
{
    IndexCount = static_cast<uint32>(indices.Num());

    Desc.ByteWidth = sizeof(uint32) * IndexCount;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    Desc.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA sub = {};
    sub.pSysMem = indices.GetData();

    if (!InGraphicDevice->CreateGPUBuffer(&Desc, &sub, Buffer.GetAddressOf()))
        assert(false &&"indices buffer create fail!!");

    return true;
}

void FIndexBuffer::Bind(FGraphicsDevice* InGraphicDevice)
{
    InGraphicDevice->BindIndexBuffer(Buffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}
