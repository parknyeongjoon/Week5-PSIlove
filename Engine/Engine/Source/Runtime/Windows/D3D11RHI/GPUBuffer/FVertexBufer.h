#pragma once
#include "FGPUBuffer.h"
#include "Container/Array.h"
#include "D3D11RHI/GraphicDevice.h"

class FVertexBufer : public FGPUBuffer
{
public:
    FVertexBufer();
    virtual ~FVertexBufer();

    template <typename T>
    bool CreateNonDyanmic(FGraphicsDevice* InGraphicDevice, const TArray<T>& vertexes);

    template <typename T>
    bool CreateDynamic(FGraphicsDevice* InGraphicDevice, const TArray<T>& vertexes);
    void Bind(FGraphicsDevice* InGraphicDevice);
private:
    uint32 Offset;
    uint32 Stride;
};

template <typename T>
bool FVertexBufer::CreateNonDyanmic(FGraphicsDevice* InGraphicDevice, const TArray<T>& vertexes)
{
    Desc.ByteWidth = sizeof(T) * vertexes.Size();
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.Usage = D3D11_USAGE_DEFAULT;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = vertexes.Data();

    if (InGraphicDevice->CreateGPUBuffer(&Desc, &subresource_data, Buffer.GetAddressOf()) == false)
        return false;

    return true;
}

template <typename T>
bool FVertexBufer::CreateDynamic(FGraphicsDevice* InGraphicDevice, const TArray<T>& vertexes)
{
    Desc.ByteWidth = sizeof(T) * vertexes.Size();
    Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresource_data = {};
    subresource_data.pSysMem = vertexes.Data();

    if (InGraphicDevice->CreateGPUBuffer(&Desc, &subresource_data, Buffer.GetAddressOf()) == false)
        return false;

    return true;
}


