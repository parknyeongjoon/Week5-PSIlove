#include "FVIBuffers.h"

void FVIBuffers::Bind(const Microsoft::WRL::ComPtr<ID3D11DeviceContext>& context) const
{
    context->IASetVertexBuffers(0, 1, VertexBuffer.GetAddressOf(), &Stride, &Offset);
    context->IASetIndexBuffer(IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
}
