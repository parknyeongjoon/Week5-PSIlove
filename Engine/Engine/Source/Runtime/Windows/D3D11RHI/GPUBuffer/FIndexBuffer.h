#pragma once
#include "FGPUBuffer.h"
#include "Container/Array.h"

class FGraphicsDevice;

class FIndexBuffer : public FGPUBuffer
{
public:
    FIndexBuffer();
    virtual ~FIndexBuffer();

    bool Create(FGraphicsDevice* InGraphicDevice, const TArray<uint32>& indices);
    void Bind(FGraphicsDevice* InGraphicDevice);

    inline uint32 GetIndexCount() const { return IndexCount; }

private:
    uint32 IndexCount;
};
