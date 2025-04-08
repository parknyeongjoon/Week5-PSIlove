#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "Container/Array.h"

#include "Container/Map.h"
#include "Container/String.h"

enum class EShaderStage;

struct FConstantBufferInfo
{
    FString Name;
    uint32 ByteWidth;
    uint32 BindSlot;
};

class FGraphicsDevice
{
public:
    ID3D11Device* Device = nullptr;
    ID3D11DeviceContext* DeviceContext = nullptr;
    IDXGISwapChain* SwapChain = nullptr;
    
    TMap<FString, ID3D11Texture2D*> FrameBuffers;
    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11Texture2D* UUIDFrameBuffer = nullptr;

    TMap<FString, ID3D11RenderTargetView*> RenderTargetViews;
    ID3D11RenderTargetView* RTVs[2] = {};
    
    DXGI_SWAP_CHAIN_DESC SwapchainDesc = {};
    
    UINT screenWidth = 0;
    UINT screenHeight = 0;
    
    // Depth-Stencil 관련 변수
    ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
    ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)
    
    void Initialize(HWND hWindow);
    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(HWND hWindow);
    bool CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) const;
    bool CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) const;
    bool CreateBlendState(const D3D11_BLEND_DESC* pBlendState, ID3D11BlendState** ppBlendState) const;
    void CreateFrameBuffer();
    void SwapBuffer() const;
    void Prepare() const;
    void Prepare(D3D11_VIEWPORT* viewport);
    void OnResize(HWND hWindow);
    void BindSampler(EShaderStage stage, uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const;
    void BindSamplers(uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const;

    void Release();
    void ReleaseDeviceAndSwapChain();
    void ReleaseFrameBuffer();
    void ReleaseDepthStencilBuffer();

    ID3D11RasterizerState* GetCurrentRasterizer() { return CurrentRasterizer; }
    inline void ChangeDepthStencilState(ID3D11DepthStencilState* newDetptStencil) const
    {
        DeviceContext->OMSetDepthStencilState(newDetptStencil, 0);
    }

    //uint32 GetPixelUUID(POINT pt);
    //uint32 DecodeUUIDColor(FVector4 UUIDColor);

public:
    bool CreateVertexShader(const FString& fileName, ID3DBlob** ppCode, ID3D11VertexShader** ppVertexShader) const;
    bool CreatePixelShader(const FString& fileName, ID3DBlob** ppCode, ID3D11PixelShader** ppPixelShader) const;
    static TArray<FConstantBufferInfo> ExtractConstantBufferNames(ID3DBlob* shaderBlob);

private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
};

