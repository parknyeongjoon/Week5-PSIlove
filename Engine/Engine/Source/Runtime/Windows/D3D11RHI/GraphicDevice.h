#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>

#include "Define.h"
#include "EngineBaseTypes.h"

#include "Core/HAL/PlatformType.h"
#include "Core/Math/Vector4.h"

class FGraphicsDevice {
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11Texture2D* UUIDFrameBuffer = nullptr;
    ID3D11RenderTargetView* RTVs[2];
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
    ID3D11RenderTargetView* UUIDFrameBufferRTV = nullptr;
    ID3D11RasterizerState* RasterizerStateSOLID = nullptr;
    ID3D11RasterizerState* RasterizerStateWIREFRAME = nullptr;
    DXGI_SWAP_CHAIN_DESC SwapchainDesc;

    
    UINT screenWidth = 0;
    UINT screenHeight = 0;
    // Depth-Stencil 관련 변수
    ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
    ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    ID3D11DepthStencilState* DepthStencilState = nullptr;
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)

    ID3D11DepthStencilState* DepthStateDisable = nullptr;

    void Initialize(HWND hWindow);
    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(HWND hWindow);
    void CreateDepthStencilState();
    void CreateRasterizerState();
    void ReleaseDeviceAndSwapChain();
    void CreateFrameBuffer();
    void ReleaseFrameBuffer();
    void ReleaseRasterizerState();
    void ReleaseDepthStencilResources();
    void Release();
    void SwapBuffer();
    void Prepare();
    void Prepare(D3D11_VIEWPORT* viewport);
    void OnResize(HWND hWindow);
    ID3D11RasterizerState* GetCurrentRasterizer() const { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);
    void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil);

    bool CreateGPUBuffer(const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData, ID3D11Buffer** ppBuffer) const;
    void BindVertexBuffer(const uint32 InStartSlot, const uint32 InNumBuffers, ID3D11Buffer* const* ppVertexBuffers, const uint32* pStrides, const uint32* pOffsets) const;
    void BindIndexBuffer(ID3D11Buffer* pIndexBuffer, const DXGI_FORMAT InFormat, const uint32 Offset) const;
    void SetDataToGPUBuffer(ID3D11Buffer* pBuffer, const void* pData, const uint32 InSize) const;
    void BindConstantBuffer(const EShaderStage InShaderStage, const uint32 InSlot, ID3D11Buffer* pBuffer) const;
    
    
    uint32 GetPixelUUID(POINT pt);
    uint32 DecodeUUIDColor(FVector4 UUIDColor);
private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
};

