#pragma once
#include "FGPUBuffer.h"
#include "HAL/PlatformType.h"
#include "Define.h"
#include "D3D11RHI/GraphicDevice.h"

class FConstantBuffer : public FGPUBuffer
{
public:
    bool Create(const FGraphicsDevice* InGraphicDevice, uint32 InSize, const void* InData, uint32 InSlotNum);
    void SetData(const FGraphicsDevice* InGraphicDevice, const void* InData) const;
    void Bind(const FGraphicsDevice* InGraphicDevice, EShaderStage InShaderStage) const;

private:
    uint32 Size = 0;
    uint32 SlotNum = -1;
};
