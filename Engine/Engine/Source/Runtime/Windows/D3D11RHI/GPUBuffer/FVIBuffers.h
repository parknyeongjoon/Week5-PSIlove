#pragma once
#include "HAL/PlatformType.h"

#define _TCHAR_DEFINED
#include <wrl.h>
#include "d3d11.h"

class FVIBuffers
{
public:
    FVIBuffers()
        : VertexBuffer(nullptr), IndexBuffer(nullptr) {}
    
    void SetVertexBuffer(const Microsoft::WRL::ComPtr<ID3D11Buffer>& InVertexBuffer, const uint32 InStride) { VertexBuffer = InVertexBuffer; Stride = InStride; }
    void SetIndexBuffer(const Microsoft::WRL::ComPtr<ID3D11Buffer>& InIndexBuffer, const uint32 InNumIndices) { IndexBuffer = InIndexBuffer; numIndices = InNumIndices; }
    Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const { return VertexBuffer; }
    Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer() const { return IndexBuffer; }

    uint32 GetNumIndices() const { return numIndices; }
    uint32 GetStride() const { return Stride; }
    uint32 GetOffset() const { return Offset; }
    
    void Bind(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context) const;
private:
    Microsoft::WRL::ComPtr<ID3D11Buffer> VertexBuffer = nullptr;
    uint32 Stride = 0;
    uint32 Offset = 0;

    Microsoft::WRL::ComPtr<ID3D11Buffer> IndexBuffer = nullptr;
    uint32 numIndices = 0;
};
