#include "ViewportClient.h"

#include "UnrealClient.h"

D3D11_VIEWPORT* FViewportClient::GetD3DViewport()
{
    return Viewport->GetViewport();
}
