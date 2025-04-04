#pragma once
#include "D3D11RHI/GraphicDevice.h"

class ISceneProxy
{
public:
    virtual void Render(ID3D11DeviceContext* context);
};
