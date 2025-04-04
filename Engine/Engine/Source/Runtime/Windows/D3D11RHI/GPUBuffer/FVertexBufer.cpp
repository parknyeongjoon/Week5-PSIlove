#include "FVertexBufer.h"

void FVertexBufer::Bind(FGraphicsDevice* InGraphicDevice)
{
    InGraphicDevice->BindVertexBuffer(0, 1, Buffer.GetAddressOf(), &Stride, &Offset);
}
