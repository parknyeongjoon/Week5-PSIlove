#include "GraphicDevice.h"
#include <wchar.h>
void FGraphicsDevice::Initialize(HWND hWindow) {
    CreateDeviceAndSwapChain(hWindow);
    CreateDefaultSampler();
    CreateFrameBuffer();
    CreateGBuffer();
    CreateDepthStencilBuffer(hWindow);
    CreateDepthStencilState();
    CreateRasterizerState();
    CurrentRasterizer = RasterizerStateSOLID;

    CreateWorldTexture();
}

void FGraphicsDevice::CreateDeviceAndSwapChain(HWND hWindow) {
    // 지원하는 Direct3D 기능 레벨을 정의
    D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

    // 스왑 체인 설정 구조체 초기화
    SwapchainDesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
    SwapchainDesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
    SwapchainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
    SwapchainDesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
    SwapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
    SwapchainDesc.BufferCount = 2; // 더블 버퍼링
    SwapchainDesc.OutputWindow = hWindow; // 렌더링할 창 핸들
    SwapchainDesc.Windowed = TRUE; // 창 모드
    SwapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

    // 디바이스와 스왑 체인 생성
    HRESULT hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
        featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
        &SwapchainDesc, &SwapChain, &Device, nullptr, &DeviceContext);

    if (FAILED(hr)) {
        MessageBox(hWindow, L"CreateDeviceAndSwapChain failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // 스왑 체인 정보 가져오기 (이후에 사용을 위해)
    SwapChain->GetDesc(&SwapchainDesc);
    screenWidth = SwapchainDesc.BufferDesc.Width;
    screenHeight = SwapchainDesc.BufferDesc.Height;
}

void FGraphicsDevice::CreateDefaultSampler()
{
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Device->CreateSamplerState(&samplerDesc, &DefaultSampler);
}

void FGraphicsDevice::CreateDepthStencilBuffer(HWND hWindow) {


    RECT clientRect;
    GetClientRect(hWindow, &clientRect);
    UINT width = clientRect.right - clientRect.left;
    UINT height = clientRect.bottom - clientRect.top;

    // 깊이/스텐실 텍스처 생성
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = width; // 텍스처 너비 설정
    descDepth.Height = height; // 텍스처 높이 설정
    descDepth.MipLevels = 1; // 미맵 레벨 수 (1로 설정하여 미맵 없음)
    descDepth.ArraySize = 1; // 텍스처 배열의 크기 (1로 단일 텍스처)
    //descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24비트 깊이와 8비트 스텐실을 위한 포맷
    descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;
    descDepth.SampleDesc.Count = 1; // 멀티샘플링 설정 (1로 단일 샘플)
    descDepth.SampleDesc.Quality = 0; // 샘플 퀄리티 설정
    descDepth.Usage = D3D11_USAGE_DEFAULT; // 텍스처 사용 방식
    //descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 깊이 스텐실 뷰로 바인딩 설정
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    descDepth.CPUAccessFlags = 0; // CPU 접근 방식 설정
    descDepth.MiscFlags = 0; // 기타 플래그 설정

    HRESULT hr = Device->CreateTexture2D(&descDepth, NULL, &DepthStencilBuffer);

    if (FAILED(hr)) {
        MessageBox(hWindow, L"Failed to create depth stencilBuffer!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }


    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 깊이 스텐실 포맷
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D; // 뷰 타입 설정 (2D 텍스처)
    descDSV.Texture2D.MipSlice = 0; // 사용할 미맵 슬라이스 설정

    hr = Device->CreateDepthStencilView(DepthStencilBuffer, // Depth stencil texture
        &descDSV, // Depth stencil desc
        &DepthStencilView);  // [out] Depth stencil view

    if (FAILED(hr)) {
        wchar_t errorMsg[256];
        swprintf_s(errorMsg, L"Failed to create depth stencil view! HRESULT: 0x%08X", hr);
        MessageBox(hWindow, errorMsg, L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // 깊이용 쉐이더 리소스 뷰 만들기
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    hr = Device->CreateShaderResourceView(DepthStencilBuffer, &srvDesc, &DepthStencilSRV);
    if (FAILED(hr)) {
        MessageBox(hWindow, L"Failed to create depth stencil SRV!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }
}

void FGraphicsDevice::CreateDepthStencilState()
{
    // DepthStencil 상태 설명 설정
    D3D11_DEPTH_STENCIL_DESC dsDesc;

    // Depth test parameters
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

    // Stencil test parameters
    //dsDesc.StencilEnable = true;
    dsDesc.StencilEnable = false;  // 스텐실 테스트 비활성화
    dsDesc.StencilReadMask = 0xFF;
    dsDesc.StencilWriteMask = 0xFF;

    // Stencil operations if pixel is front-facing
    dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //// DepthStencil 상태 생성
    HRESULT hr = Device->CreateDepthStencilState(&dsDesc, &DepthStencilState);
    if (FAILED(hr)) {
        // 오류 처리
        return;
    }

    D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
    depthStencilDesc.DepthEnable = FALSE;  // 깊이 테스트 유지
    depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;  // 깊이 버퍼에 쓰지 않음
    depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;  // 깊이 비교를 항상 통과
    Device->CreateDepthStencilState(&depthStencilDesc, &DepthStateDisable);

}

void FGraphicsDevice::CreateRasterizerState()
{
    D3D11_RASTERIZER_DESC rasterizerdesc = {};
    rasterizerdesc.FillMode = D3D11_FILL_SOLID;
    rasterizerdesc.CullMode = D3D11_CULL_BACK;
    Device->CreateRasterizerState(&rasterizerdesc, &RasterizerStateSOLID);

    rasterizerdesc.FillMode = D3D11_FILL_WIREFRAME;
    rasterizerdesc.CullMode = D3D11_CULL_BACK;
    Device->CreateRasterizerState(&rasterizerdesc, &RasterizerStateWIREFRAME);
}


void FGraphicsDevice::ReleaseDeviceAndSwapChain()
{
    if (DeviceContext)
    {
        DeviceContext->Flush(); // 남아있는 GPU 명령 실행
    }

    SAFE_RELEASE(SwapChain)
    SAFE_RELEASE(Device)
    SAFE_RELEASE(DeviceContext)
}

void FGraphicsDevice::CreateFrameBuffer()
{
    // 스왑 체인으로부터 백 버퍼 텍스처 가져오기
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

    // 렌더 타겟 뷰 생성
    D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
    framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
    framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);

    // 핑퐁 텍스처 생성
    D3D11_TEXTURE2D_DESC pingpongTextureDesc = {};
    pingpongTextureDesc.Width = screenWidth;
    pingpongTextureDesc.Height = screenHeight;
    pingpongTextureDesc.MipLevels = 1;
    pingpongTextureDesc.ArraySize = 1;
    pingpongTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    pingpongTextureDesc.SampleDesc.Count = 1;
    pingpongTextureDesc.Usage = D3D11_USAGE_DEFAULT;
    pingpongTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    pingpongTextureDesc.CPUAccessFlags = 0;
    pingpongTextureDesc.MiscFlags = 0;

    // Depth Texture 생성용
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = screenWidth;
    depthDesc.Height = screenHeight;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    depthDesc.CPUAccessFlags = 0;
    depthDesc.MiscFlags = 0;

    // Create DSV
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

    // Create SRV for depth
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    Device->CreateSamplerState(&samplerDesc, &SamplerState);
    
    for (int i = 0;i < 2; i++)
    {
        // SceneColor (pingpong)
        Device->CreateTexture2D(&pingpongTextureDesc, nullptr, &pingpongTex[i]);
        Device->CreateRenderTargetView(pingpongTex[i], nullptr, &pingpongRTV[i]);
        Device->CreateShaderResourceView(pingpongTex[i], nullptr, &pingpongSRV[i]);
    }

//  RTVs[0] = FrameBufferRTV;
    RTVs[0] = GetWriteRTV();
}

void FGraphicsDevice::CreateGBuffer()
{
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = screenWidth;
    textureDesc.Height = screenHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    textureDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
    Device->CreateTexture2D(&textureDesc, nullptr, &PositionBuffer);
    textureDesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM;
    Device->CreateTexture2D(&textureDesc, nullptr, &NormalBuffer);
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Device->CreateTexture2D(&textureDesc, nullptr, &DiffuseBuffer);
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    Device->CreateTexture2D(&textureDesc, nullptr, &MaterialBuffer);
    
    // 렌더 타겟 뷰 생성
    D3D11_RENDER_TARGET_VIEW_DESC GBufferRTVdesc = {};
    GBufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처
    
    GBufferRTVdesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT; // 색상 포맷
    Device->CreateRenderTargetView(PositionBuffer, &GBufferRTVdesc, &PositionRTV);
    GBufferRTVdesc.Format = DXGI_FORMAT_R10G10B10A2_UNORM; // 색상 포맷
    Device->CreateRenderTargetView(NormalBuffer, &GBufferRTVdesc, &NormalRTV);
    GBufferRTVdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 색상 포맷
    Device->CreateRenderTargetView(DiffuseBuffer, &GBufferRTVdesc, &DiffuseRTV);
    GBufferRTVdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 색상 포맷
    Device->CreateRenderTargetView(MaterialBuffer, &GBufferRTVdesc, &MaterialRTV);

    RTVs[1] = PositionRTV;
    RTVs[2] = NormalRTV;
    RTVs[3] = DiffuseRTV;
    RTVs[4] = MaterialRTV;

    CreateGBufferSRVs();
}

void FGraphicsDevice::CreateGBufferSRVs()
{
    HRESULT hr = Device->CreateShaderResourceView(PositionBuffer, nullptr, &PositionSRV);
    if (FAILED(hr)) return;
    GBufferSRVs[0] = PositionSRV;

    // NormalBuffer에서 SRV 생성
    hr = Device->CreateShaderResourceView(NormalBuffer, nullptr, &NormalSRV);
    if (FAILED(hr)) return;
    GBufferSRVs[1] = NormalSRV;

    // DiffuseBuffer에서 SRV 생성
    hr = Device->CreateShaderResourceView(DiffuseBuffer, nullptr, &DiffuseSRV);
    if (FAILED(hr)) return;
    GBufferSRVs[2] = DiffuseSRV;

    // MaterialBuffer에서 SRV 생성
    hr = Device->CreateShaderResourceView(MaterialBuffer, nullptr, &MaterialSRV);
    if (FAILED(hr)) return;
    GBufferSRVs[3] = MaterialSRV;
}

void FGraphicsDevice::ReleaseFrameBuffer()
{
    SAFE_RELEASE(FrameBuffer)
    SAFE_RELEASE(FrameBufferRTV)

    for (int i = 0; i < 2; i++)
    {
        if (pingpongTex[i])
        {
            pingpongTex[i]->Release();
            pingpongTex[i] = nullptr;
        }
        if (pingpongRTV[i])
        {
            pingpongRTV[i]->Release();
            pingpongRTV[i] = nullptr;
        }
        if (pingpongSRV[i])
        {
            pingpongSRV[i]->Release();
            pingpongSRV[i] = nullptr;
        }
    }
}

void FGraphicsDevice::ReleaseGBuffer()
{
    SAFE_RELEASE(PositionBuffer)
    SAFE_RELEASE(NormalBuffer)
    SAFE_RELEASE(DiffuseBuffer)
    SAFE_RELEASE(MaterialBuffer)
    SAFE_RELEASE(PositionRTV)
    SAFE_RELEASE(NormalRTV)
    SAFE_RELEASE(DiffuseRTV)
    SAFE_RELEASE(MaterialRTV)
    SAFE_RELEASE(PositionSRV)
}

void FGraphicsDevice::ReleaseGBufferSRVs()
{
    SAFE_RELEASE(PositionSRV)
    SAFE_RELEASE(NormalSRV)
    SAFE_RELEASE(DiffuseSRV)
    SAFE_RELEASE(MaterialSRV)
}

void FGraphicsDevice::ReleaseRasterizerState()
{
    SAFE_RELEASE(RasterizerStateSOLID)
    SAFE_RELEASE(RasterizerStateWIREFRAME)
}

void FGraphicsDevice::ReleaseDepthStencilResources()
{
    SAFE_RELEASE(DepthStencilBuffer)
    SAFE_RELEASE(DepthStencilView)
    SAFE_RELEASE(DepthStencilState)
    SAFE_RELEASE(DepthStateDisable)
}

void FGraphicsDevice::Release() 
{
    ReleaseRasterizerState();
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseFrameBuffer();
    ReleaseDepthStencilResources();
    ReleaseDeviceAndSwapChain();
}

void FGraphicsDevice::SwapBuffer() const
{
    SwapChain->Present(1, 0);
}

void FGraphicsDevice::ClearRenderTarget()
{
    CurrentIndex = 0;
    DeviceContext->ClearRenderTargetView(pingpongRTV[0], ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearRenderTargetView(pingpongRTV[1], ClearColor);
    DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearRenderTargetView(PositionRTV, PositionClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearRenderTargetView(NormalRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearRenderTargetView(DiffuseRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearRenderTargetView(MaterialRTV, ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가
}


void FGraphicsDevice::Prepare()
{
    CurrentIndex = 0;
    DeviceContext->OMSetDepthStencilState(DepthStencilState, 0);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
    DeviceContext->OMSetRenderTargets(5, RTVs, DepthStencilView); // 렌더 타겟 설정
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::PrepareGizmo()
{
    auto* RTV = GetWriteRTV();
    DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(RasterizerStateSOLID); //레스터 라이저 상태 설정
    DeviceContext->OMSetRenderTargets(1, &RTV, DepthStencilView); // 렌더 타겟 설정
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void FGraphicsDevice::PrepareLighting() const
{
    auto* RenderTarget = GetWriteRTV();
    DeviceContext->OMSetDepthStencilState(nullptr, 0);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
    DeviceContext->OMSetRenderTargets(1, &RenderTarget, nullptr);
    DeviceContext->PSSetShaderResources(0, 4, GBufferSRVs);
    DeviceContext->PSSetSamplers(0, 1, &SamplerState);
}

void FGraphicsDevice::PrepareDepthScene() const
{
    ID3D11RenderTargetView* writeRTV = GetWriteRTV();
    DeviceContext->OMSetDepthStencilState(nullptr, 0);
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
    DeviceContext->OMSetRenderTargets(1, &writeRTV, nullptr);
    DeviceContext->PSSetShaderResources(0, 1, &DepthStencilSRV);
    DeviceContext->PSSetSamplers(0, 1, &SamplerState);
}

void FGraphicsDevice::PreparePostProcessRender()
{
    SwapRTV();
    auto* RenderTarget = GetWriteRTV();
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(RasterizerStateSOLID); //레스터 라이저 상태 설정
    DeviceContext->OMSetDepthStencilState(nullptr, 0);
    DeviceContext->OMSetRenderTargets(1, &RenderTarget, nullptr); // 렌더 타겟 설정
}

void FGraphicsDevice::PrepareGridRender()
{
    auto* RenderTarget = GetWriteRTV();
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
    DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
    DeviceContext->OMSetRenderTargets(1, &RenderTarget, DepthStencilView); // 렌더 타겟 설정
}

void FGraphicsDevice::PrepareFinalRender()
{
    SwapRTV();
    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정
    DeviceContext->RSSetState(RasterizerStateSOLID); //레스터 라이저 상태 설정
    DeviceContext->OMSetDepthStencilState(nullptr, 0);
    DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr); // 렌더 타겟 설정(백버퍼를 가르킴)
}


void FGraphicsDevice::OnResize(HWND hWindow) {
    DeviceContext->OMSetRenderTargets(0, RTVs, 0);
    
    ReleaseFrameBuffer();
    ReleaseGBuffer();
    SAFE_RELEASE(DepthStencilBuffer)
    SAFE_RELEASE(DepthStencilView)

    if (screenWidth == 0 || screenHeight == 0) {
        MessageBox(hWindow, L"Invalid width or height for ResizeBuffers!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // SwapChain 크기 조정
    HRESULT hr;
    hr = SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_B8G8R8A8_UNORM, 0);  // DXGI_FORMAT_B8G8R8A8_UNORM으로 시도
    if (FAILED(hr)) {
        MessageBox(hWindow, L"failed", L"ResizeBuffers failed ", MB_ICONERROR | MB_OK);
        return;
    }
    
    SwapChain->GetDesc(&SwapchainDesc);
    screenWidth = SwapchainDesc.BufferDesc.Width;
    screenHeight = SwapchainDesc.BufferDesc.Height;

    CreateFrameBuffer();
    CreateGBuffer();
    CreateDepthStencilBuffer(hWindow);
}


void FGraphicsDevice::ChangeRasterizer(EViewModeIndex evi)
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Wireframe:
        CurrentRasterizer = RasterizerStateWIREFRAME;
        break;
    case EViewModeIndex::VMI_Lit:
        CurrentRasterizer = RasterizerStateSOLID;
        break;
    case EViewModeIndex::VMI_Unlit:
        CurrentRasterizer = RasterizerStateSOLID;
        break;
    case EViewModeIndex::VMI_SceneDepth:
        CurrentRasterizer = RasterizerStateSOLID;
        break;
    }
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
}

void FGraphicsDevice::CreateWorldTexture()
{
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = screenWidth;
    textureDesc.Height = screenHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = Device->CreateTexture2D(&textureDesc, nullptr, &WorldTexture);
    if (FAILED(hr))
    {
        return;
    }

    D3D11_RENDER_TARGET_VIEW_DESC UUIDFrameBufferRTVDesc = {};
    UUIDFrameBufferRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // 색상 포맷
    UUIDFrameBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    hr = Device->CreateRenderTargetView(WorldTexture, &UUIDFrameBufferRTVDesc, &WorldTextureRTV);
    if (FAILED(hr))
    {
        return;
    }

    hr = Device->CreateShaderResourceView(WorldTexture, nullptr, &WorldTextureSRV);
    if (FAILED(hr))
    {
        return;
    }
}