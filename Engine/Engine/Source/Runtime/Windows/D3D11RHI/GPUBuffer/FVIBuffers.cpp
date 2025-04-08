#include "FVIBuffers.h"

void FVIBuffers::Bind(ID3D11DeviceContext* context) const
{
    context->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);
    context->IASetPrimitiveTopology(Topology);
    
    if (IndexBuffer != nullptr)
        context->IASetIndexBuffer(IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
}
