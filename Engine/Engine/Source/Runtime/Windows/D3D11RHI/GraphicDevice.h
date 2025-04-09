#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#define SAFE_RELEASE(p) if(p) { p->Release(); p = nullptr; }
#include <d3d11.h>
#include "EngineBaseTypes.h"
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
    
    ID3D11Texture2D* FrameBuffer = nullptr;
    ID3D11Texture2D* PositionBuffer = nullptr;
    ID3D11Texture2D* NormalBuffer = nullptr;
    ID3D11Texture2D* DiffuseBuffer = nullptr;
    ID3D11Texture2D* MaterialBuffer = nullptr;

    ID3D11ShaderResourceView* GBufferSRVs[4] = {};
    ID3D11ShaderResourceView* PositionSRV = nullptr;
    ID3D11ShaderResourceView* NormalSRV = nullptr;
    ID3D11ShaderResourceView* DiffuseSRV = nullptr;
    ID3D11ShaderResourceView* MaterialSRV = nullptr;
        
    ID3D11RenderTargetView* RTVs[5] = { };
    ID3D11RenderTargetView* FrameBufferRTV = nullptr;
    ID3D11RenderTargetView* PositionRTV = nullptr;
    ID3D11RenderTargetView* NormalRTV = nullptr;
    ID3D11RenderTargetView* DiffuseRTV = nullptr;
    ID3D11RenderTargetView* MaterialRTV = nullptr;

    // post-processing
    ID3D11Texture2D* pingpongTex[2];
    ID3D11RenderTargetView* pingpongRTV[2];
    ID3D11ShaderResourceView* pingpongSRV[2];

    // sampler
    ID3D11SamplerState* SamplerState = nullptr;

    int CurrentIndex = 0;
    void SwapRTV() { CurrentIndex = 1 - CurrentIndex; }

    ID3D11RenderTargetView* GetWriteRTV() const { return pingpongRTV[CurrentIndex]; }
    ID3D11ShaderResourceView* GetReadSRV() const { return pingpongSRV[1 - CurrentIndex]; }

    DXGI_SWAP_CHAIN_DESC SwapchainDesc;
    
    UINT screenWidth = 0;
    UINT screenHeight = 0;
    // Depth-Stencil 관련 변수
    ID3D11Texture2D* DepthStencilBuffer = nullptr;  // 깊이/스텐실 텍스처
    ID3D11DepthStencilView* DepthStencilView = nullptr;  // 깊이/스텐실 뷰
    ID3D11ShaderResourceView* DepthStencilSRV = nullptr;  // 깊이/스텐실 SRV
    ID3D11DepthStencilState* DepthStencilState = nullptr;
    FLOAT ClearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f }; // 화면을 초기화(clear) 할 때 사용할 색상(RGBA)
    FLOAT PositionClearColor[4] = {0,0,0,0};

    void Initialize(HWND hWindow);
    void CreateDeviceAndSwapChain(HWND hWindow);
    void CreateDepthStencilBuffer(HWND hWindow);
    bool CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) const;
    bool CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) const;
    bool CreateBlendState(const D3D11_BLEND_DESC* pBlendState, ID3D11BlendState** ppBlendState) const;
    void CreateFrameBuffer();
    void CreateGBuffer();
    void CreateGBufferSRVs();
    void ReleaseFrameBuffer();
    void ReleaseGBuffer();
    void ReleaseGBufferSRVs();
    void ReleaseDepthStencilResources();
    void Release();
    
    void ClearRenderTarget();
    void SwapBuffer() const;
    void Prepare();
    void PrepareGizmo();
    void PrepareLighting() const;
    void PrepareDepthScene() const;
    // void Prepare(D3D11_VIEWPORT* viewport) const;
    //void Prepare() const;
    //void Prepare(D3D11_VIEWPORT* viewport);
    void OnResize(HWND hWindow);
    void BindSampler(EShaderStage stage, uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const;
    void BindSamplers(uint32 StartSlot, uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const;
    
    void ReleaseDeviceAndSwapChain();
    ID3D11RasterizerState* GetCurrentRasterizer() const { return CurrentRasterizer; }
    void ChangeRasterizer(EViewModeIndex evi);

    ID3D11Texture2D* WorldTexture = nullptr;
    ID3D11RenderTargetView* WorldTextureRTV = nullptr;

    void CreateWorldTexture();
    ID3D11ShaderResourceView* WorldTextureSRV = nullptr;

    ID3D11SamplerState* LinearSampler = nullptr;
private:
    ID3D11RasterizerState* CurrentRasterizer = nullptr;
public:
    bool CreateVertexShader(const FString& fileName, ID3DBlob** ppCode, ID3D11VertexShader** ppVertexShader) const;
    bool CreatePixelShader(const FString& fileName, ID3DBlob** ppCode, ID3D11PixelShader** ppPixelShader) const;
    static TArray<FConstantBufferInfo> ExtractConstantBufferNames(ID3DBlob* shaderBlob);
};

