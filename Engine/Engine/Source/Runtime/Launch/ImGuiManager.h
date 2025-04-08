#pragma once
#define _TCHAR_DEFINED
#include <wrl.h>
#include <d3d11.h>

class UImGuiManager
{
public:
    void Initialize(HWND hWnd, Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext);
    void BeginFrame();
    void EndFrame();
    void PreferenceStyle();
    void Shutdown();
};

