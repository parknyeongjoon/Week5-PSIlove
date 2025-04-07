#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include <wrl.h>

#include "EngineBaseTypes.h"
#include "Container/Array.h"

#include "Container/Map.h"
#include "Container/String.h"

struct FConstantBufferInfo
{
    FString Name;
    uint32 ByteWidth;
    uint32 BindSlot;
};

class FGraphicsDevice
{
public:
    Microsoft::WRL::ComPtr<ID3D11Device> Device = nullptr;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> DeviceContext = nullptr;
    Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain = nullptr;
    TMap<FString, Microsoft::WRL::ComPtr<ID3D11Texture2D>> FrameBuffers;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> FrameBuffer = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> UUIDFrameBuffer = nullptr;

    TMap<FString, Microsoft::WRL::ComPtr<ID3D11RenderTargetView>> RenderTargetViews;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> RTVs[2];
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> FrameBufferRTV = nullptr;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> UUIDFrameBufferRTV = nullptr;

    TMap<FString, Microsoft::WRL::ComPtr<ID3D11RasterizerState>> DepthStencilViews;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStateSOLID = nullptr;
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> RasterizerStateWIREFRAME = nullptr;
    
    DXGI_SWAP_CHAIN_DESC SwapchainDesc;
    
    UINT screenWidth = 0;
    UINT screenHeight = 0;
    
    // Depth-Stencil 관련 변수
    TMap<FString, Microsoft::WRL::ComPtr<ID3D11DepthStencilState>> DepthStencilStates;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStencilState = nullptr;
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)

    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStateDisable = nullptr;

    void Initialize(HWND hWindow);
    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(HWND hWindow);
    void CreateDepthStencilState();
    void CreateRasterizerState();
    void CreateFrameBuffer();
    void SwapBuffer();
    void Prepare();
    void Prepare(D3D11_VIEWPORT* viewport);
    void OnResize(HWND hWindow);
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> GetCurrentRasterizer() { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);
    inline void ChangeDepthStencilState(const Microsoft::WRL::ComPtr<ID3D11DepthStencilState>& newDetptStencil) const
    {
        DeviceContext->OMSetDepthStencilState(newDetptStencil.Get(), 0);
    }

    //uint32 GetPixelUUID(POINT pt);
    //uint32 DecodeUUIDColor(FVector4 UUIDColor);

public:
    bool CreateVertexShader(const FString& fileName, ID3DBlob** ppCode, ID3D11VertexShader** ppVertexShader) const;
    bool CreatePixelShader(const FString& fileName, ID3DBlob** ppCode, ID3D11PixelShader** ppPixelShader) const;
    static TArray<FConstantBufferInfo> ExtractConstantBufferNames(ID3DBlob* shaderBlob);

private:
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> CurrentRasterizer = nullptr;
};

