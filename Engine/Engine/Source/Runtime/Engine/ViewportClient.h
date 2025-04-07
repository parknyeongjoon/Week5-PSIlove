#pragma once
#include "HAL/PlatformType.h"

class FViewport;
class ULevel;
class D3D11_VIEWPORT;


class FViewportClient
{
public:
    FViewportClient()
        : Viewport(nullptr) {}
    virtual ~FViewportClient() = default;

    // FViewport에서 발생하는 이벤트를 처리하는 가상 함수들
    //virtual void OnInput(const FInputEvent& Event) = 0;
    virtual void Draw(FViewport* Viewport) = 0;
    virtual ULevel* GetLevel() const { return NULL; }
    // FViewport에 대한 참조 (혹은 소유)

public:
    FViewport* GetViewport() const { return Viewport; }
    D3D11_VIEWPORT* GetD3DViewport();

    int32 GetViewportIndex() const { return ViewportIndex; }
protected: 
    FViewport* Viewport;
    int32 ViewportIndex;
};
