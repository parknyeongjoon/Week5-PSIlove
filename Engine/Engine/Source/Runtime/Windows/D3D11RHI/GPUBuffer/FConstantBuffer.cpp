#include "FConstantBuffer.h"

bool FConstantBuffer::Create(const FGraphicsDevice* InGraphicDevice, const uint32 InSize, const void* InData, const uint32 InSlotNum)
{
    Size = InSize;
    SlotNum = InSlotNum;
    
    Desc.ByteWidth = Size;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA SubResourceData;
    SubResourceData.pSysMem = InData;

    bool success = false;
    if (InData == nullptr)
    {
        success = InGraphicDevice->CreateGPUBuffer(&Desc, nullptr, Buffer.GetAddressOf());
    }
    else
    {
        success = InGraphicDevice->CreateGPUBuffer(&Desc, &SubResourceData, Buffer.GetAddressOf());
    }

    if (!success)
        assert(success && "Create constant buffer failed!");

    return true;
}

void FConstantBuffer::SetData(const FGraphicsDevice* InGraphicDevice, const void* InData) const
{
    InGraphicDevice->SetDataToGPUBuffer(Buffer.Get(), InData, Size);
}

void FConstantBuffer::Bind(const FGraphicsDevice* InGraphicDevice, const EShaderStage InShaderStage) const
{
    InGraphicDevice->BindConstantBuffer(InShaderStage, SlotNum, Buffer.Get());
}
