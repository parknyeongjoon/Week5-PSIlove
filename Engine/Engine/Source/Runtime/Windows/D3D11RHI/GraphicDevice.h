#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>

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

    // post-processing
    ID3D11Texture2D* pingpongTex[2];
    ID3D11RenderTargetView* pingpongRTV[2];
    ID3D11ShaderResourceView* pingpongSRV[2];

    // depth & stencil
    ID3D11Texture2D* pingpongDepthTex[2];
    ID3D11DepthStencilView* pingpongDSV[2];
    ID3D11ShaderResourceView* pingpongDepthSRV[2];

    // sampler
    ID3D11SamplerState* SamplerState = nullptr;

    int CurrentIndex = 0;
    void SwapRTV() { CurrentIndex = 1 - CurrentIndex; }

    ID3D11RenderTargetView* GetWriteRTV() const { return pingpongRTV[CurrentIndex]; }
    ID3D11ShaderResourceView* GetReadSRV() const { return pingpongSRV[1 - CurrentIndex]; }

    ID3D11DepthStencilView* GetWriteDSV() const { return pingpongDSV[CurrentIndex]; }
    ID3D11ShaderResourceView* GetReadDepthSRV() const { return pingpongDepthSRV[1 - CurrentIndex]; }

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
    void PrepareFogRender();    
    void PrepareFinalRender();
    void OnResize(HWND hWindow);
    ID3D11RasterizerState* GetCurrentRasterizer() { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);
    void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil);

    uint32 GetPixelUUID(POINT pt);
    uint32 DecodeUUIDColor(FVector4 UUIDColor);
private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
};

