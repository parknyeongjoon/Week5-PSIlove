#include "GraphicDevice.h"

#include <cassert>
#include <d3dcompiler.h>
#include <filesystem>

#include "Define.h"
#include "Renderer/Renderer.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

void FGraphicsDevice::Initialize(const HWND hWindow)
{
    CreateDeviceAndSwapChain(hWindow);
    CreateFrameBuffer();
    CreateDepthStencilBuffer(hWindow);
}
void FGraphicsDevice::CreateDeviceAndSwapChain(const HWND hWindow)
{
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

    if (FAILED(hr))
    {
        MessageBox(hWindow, L"CreateDeviceAndSwapChain failed!", L"Error", MB_ICONERROR | MB_OK);
        return;
    }

    // 스왑 체인 정보 가져오기 (이후에 사용을 위해)
    SwapChain->GetDesc(&SwapchainDesc);
    screenWidth = SwapchainDesc.BufferDesc.Width;
    screenHeight = SwapchainDesc.BufferDesc.Height;
}



void FGraphicsDevice::CreateDepthStencilBuffer(HWND hWindow)
{
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
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT; // 24비트 깊이와 8비트 스텐실을 위한 포맷
    descDepth.SampleDesc.Count = 1; // 멀티샘플링 설정 (1로 단일 샘플)
    descDepth.SampleDesc.Quality = 0; // 샘플 퀄리티 설정
    descDepth.Usage = D3D11_USAGE_DEFAULT; // 텍스처 사용 방식
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 깊이 스텐실 뷰로 바인딩 설정
    descDepth.CPUAccessFlags = 0; // CPU 접근 방식 설정
    descDepth.MiscFlags = 0; // 기타 플래그 설정

    HRESULT hr = Device->CreateTexture2D(&descDepth, NULL, &DepthStencilBuffer);

    if (FAILED(hr))
    {
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
}

bool FGraphicsDevice::CreateDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc, ID3D11DepthStencilState** ppDepthStencilState) const
{
    if (FAILED(Device->CreateDepthStencilState(pDepthStencilDesc, ppDepthStencilState)))
        return false;

    return true;
}

bool FGraphicsDevice::CreateRasterizerState(const D3D11_RASTERIZER_DESC* pRasterizerDesc, ID3D11RasterizerState** ppRasterizerState) const
{
    if (FAILED(Device->CreateRasterizerState(pRasterizerDesc, ppRasterizerState)))
        return false;

    return true;
}

void FGraphicsDevice::CreateFrameBuffer()
{
    // 스왑 체인으로부터 백 버퍼 텍스처 가져오기
    SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&FrameBuffer));

    // 렌더 타겟 뷰 생성
    D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
    framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
    framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &RTVs[0]);
    
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = screenWidth;
    textureDesc.Height = screenHeight;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    Device->CreateTexture2D(&textureDesc, nullptr, &UUIDFrameBuffer);

    D3D11_RENDER_TARGET_VIEW_DESC UUIDFrameBufferRTVDesc = {};
    UUIDFrameBufferRTVDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // 색상 포맷
    UUIDFrameBufferRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

    Device->CreateRenderTargetView(UUIDFrameBuffer, &UUIDFrameBufferRTVDesc, &RTVs[1]);
}

void FGraphicsDevice::SwapBuffer() const
{
    SwapChain->Present(1, 0);
}
void FGraphicsDevice::Prepare() const
{
    DeviceContext->ClearRenderTargetView(RTVs[0], ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearRenderTargetView(RTVs[1], ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가
    
    ID3D11RenderTargetView* pRTVs[2] = { RTVs[0], RTVs[1] };
    DeviceContext->OMSetRenderTargets(2, pRTVs, DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)}
}

void FGraphicsDevice::Prepare(D3D11_VIEWPORT* viewport)
{
    DeviceContext->ClearRenderTargetView(RTVs[0], ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가

    DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST); // 정정 연결 방식 설정

    DeviceContext->RSSetViewports(1, viewport); // GPU가 화면을 렌더링할 영역 설정
    DeviceContext->RSSetState(CurrentRasterizer); //레스터 라이저 상태 설정
    
    DeviceContext->OMSetRenderTargets(1, &RTVs[0], DepthStencilView); // 렌더 타겟 설정(백버퍼를 가르킴)
    DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}


void FGraphicsDevice::OnResize(const HWND hWindow)
{
    // 1) 파이프라인에서 모든 리소스 언바인드
    //    - RenderTarget / DepthStencil
    ID3D11RenderTargetView* nullRTV[1] = { nullptr };
    DeviceContext->OMSetRenderTargets(1, nullRTV, nullptr);

    //    - 셰이더 언바인드
    DeviceContext->VSSetShader(nullptr, nullptr, 0);
    DeviceContext->PSSetShader(nullptr, nullptr, 0);
    DeviceContext->GSSetShader(nullptr, nullptr, 0);
    DeviceContext->HSSetShader(nullptr, nullptr, 0);
    DeviceContext->DSSetShader(nullptr, nullptr, 0);
    DeviceContext->CSSetShader(nullptr, nullptr, 0);

    //    - SRV 언바인드 (모든 스테이지)
    constexpr UINT SRV_SLOTS = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
    ID3D11ShaderResourceView* nullSRV[SRV_SLOTS] = { nullptr };
    DeviceContext->VSSetShaderResources(0, SRV_SLOTS, nullSRV);
    DeviceContext->PSSetShaderResources(0, SRV_SLOTS, nullSRV);
    DeviceContext->GSSetShaderResources(0, SRV_SLOTS, nullSRV);
    DeviceContext->HSSetShaderResources(0, SRV_SLOTS, nullSRV);
    DeviceContext->DSSetShaderResources(0, SRV_SLOTS, nullSRV);
    DeviceContext->CSSetShaderResources(0, SRV_SLOTS, nullSRV);

    //    - UAV 언바인드 (컴퓨트 스테이지)
    constexpr UINT UAV_SLOTS = D3D11_PS_CS_UAV_REGISTER_COUNT;
    ID3D11UnorderedAccessView* nullUAV[UAV_SLOTS] = { nullptr };
    UINT nullCounts[UAV_SLOTS] = { 0 };
    DeviceContext->CSSetUnorderedAccessViews(0, UAV_SLOTS, nullUAV, nullCounts);

    //    - Sampler 언바인드 (모든 스테이지)
    constexpr UINT SMP_SLOTS = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
    ID3D11SamplerState* nullSMP[SMP_SLOTS] = { nullptr };
    DeviceContext->VSSetSamplers(0, SMP_SLOTS, nullSMP);
    DeviceContext->PSSetSamplers(0, SMP_SLOTS, nullSMP);
    DeviceContext->GSSetSamplers(0, SMP_SLOTS, nullSMP);
    DeviceContext->HSSetSamplers(0, SMP_SLOTS, nullSMP);
    DeviceContext->DSSetSamplers(0, SMP_SLOTS, nullSMP);
    DeviceContext->CSSetSamplers(0, SMP_SLOTS, nullSMP);

    // 2) 상태 초기화 & GPU 큐 비우기
    DeviceContext->ClearState();
    DeviceContext->Flush();

    // 3) 프레임버퍼·깊이 버퍼 등 해제
    ReleaseFrameBuffer();       
    ReleaseDepthStencilBuffer();

    // 5) 클라이언트 영역 크기 얻기
    RECT rc;
    GetClientRect(hWindow, &rc);
    UINT newW = rc.right  - rc.left;
    UINT newH = rc.bottom - rc.top;
    if (newW == 0 || newH == 0) {
        MessageBox(hWindow,
                   L"Invalid width or height for ResizeBuffers!",
                   L"Error",
                   MB_ICONERROR | MB_OK);
        return;
    }
    
    // 6) SwapChain 크기 조정
    HRESULT hr = SwapChain->ResizeBuffers(
        0,                // bufferCount 유지
        newW, newH,       // 새 폭·높이
        DXGI_FORMAT_UNKNOWN, // 기존 포맷 유지
        0                 // 기존 플래그 유지
    );

    // 7) 내부 변수 갱신
    SwapChain->GetDesc(&SwapchainDesc);
    screenWidth  = SwapchainDesc.BufferDesc.Width;
    screenHeight = SwapchainDesc.BufferDesc.Height;

    // 8) 프레임버퍼·깊이 버퍼 재생성
    CreateFrameBuffer();
    CreateDepthStencilBuffer(hWindow);
    GEngineLoop.renderer.ReBindSamplers();
}

void FGraphicsDevice::BindSampler(const EShaderStage stage, const uint32 StartSlot, const uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const
{
    if (EShaderStage::VS == stage)
        DeviceContext->VSSetSamplers(StartSlot, NumSamplers, ppSamplers);

    if (EShaderStage::HS == stage)
        DeviceContext->HSSetSamplers(StartSlot, NumSamplers, ppSamplers);

    if (EShaderStage::DS == stage)
        DeviceContext->DSSetSamplers(StartSlot, NumSamplers, ppSamplers);

    if (EShaderStage::GS == stage)
        DeviceContext->GSSetSamplers(StartSlot, NumSamplers, ppSamplers);

    if (EShaderStage::PS == stage)
        DeviceContext->PSSetSamplers(StartSlot, NumSamplers, ppSamplers);
}

void FGraphicsDevice::BindSamplers(const uint32 StartSlot, const uint32 NumSamplers, ID3D11SamplerState* const* ppSamplers) const
{
    BindSampler(EShaderStage::VS, StartSlot, NumSamplers, ppSamplers);
    //BindSampler(EShaderStage::HS, StartSlot, NumSamplers, ppSamplers);
    //BindSampler(EShaderStage::DS, StartSlot, NumSamplers, ppSamplers);
    //BindSampler(EShaderStage::GS, StartSlot, NumSamplers, ppSamplers);
    BindSampler(EShaderStage::PS, StartSlot, NumSamplers, ppSamplers);
}

void FGraphicsDevice::Release()
{
    DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ReleaseFrameBuffer();
    ReleaseDepthStencilBuffer();
    ReleaseDeviceAndSwapChain();
}

void FGraphicsDevice::ReleaseDeviceAndSwapChain()
{
    if (DeviceContext)
    {
        DeviceContext->Flush(); // 남아있는 GPU 명령 실행
    }

    if (SwapChain)
    {
        SwapChain->Release();
        SwapChain = nullptr;
    }

    if (Device)
    {
        Device->Release();
        Device = nullptr;
    }

    if (DeviceContext)
    {
        DeviceContext->Release();
        DeviceContext = nullptr;
    }
}

void FGraphicsDevice::ReleaseFrameBuffer()
{
    if (FrameBuffer)
    {
        FrameBuffer->Release();
        FrameBuffer = nullptr;
    }

    if (UUIDFrameBuffer)
    {
        UUIDFrameBuffer->Release();
        UUIDFrameBuffer = nullptr;
    }

    
    if (RTVs[0]) { RTVs[0]->Release(); RTVs[0] = nullptr; }
    if (RTVs[1]) { RTVs[1]->Release(); RTVs[1] = nullptr; }
}

void FGraphicsDevice::ReleaseDepthStencilBuffer()
{
    if (DepthStencilView)
    {
        DepthStencilView->Release();
        DepthStencilView = nullptr;
    }

    // 깊이/스텐실 버퍼 해제
    if (DepthStencilBuffer)
    {
        DepthStencilBuffer->Release();
        DepthStencilBuffer = nullptr;
    }
}

bool FGraphicsDevice::CreateBlendState(const D3D11_BLEND_DESC* pBlendState, ID3D11BlendState** ppBlendState) const
{
    if (FAILED(Device->CreateBlendState(pBlendState, ppBlendState)))
        return false;

    return true;
}

//uint32 FGraphicsDevice::GetPixelUUID(POINT pt)
//{
//    // pt.x 값 제한하기
//    if (pt.x < 0) {
//        pt.x = 0;
//    }
//    else if (pt.x > screenWidth) {
//        pt.x = screenWidth;
//    }
//
//    // pt.y 값 제한하기
//    if (pt.y < 0) {
//        pt.y = 0;
//    }
//    else if (pt.y > screenHeight) {
//        pt.y = screenHeight;
//    }
//
//    // 1. Staging 텍스처 생성 (1x1 픽셀)
//    D3D11_TEXTURE2D_DESC stagingDesc = {};
//    stagingDesc.Width = 1; // 픽셀 1개만 복사
//    stagingDesc.Height = 1;
//    stagingDesc.MipLevels = 1;
//    stagingDesc.ArraySize = 1;
//    stagingDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 원본 텍스처 포맷과 동일
//    stagingDesc.SampleDesc.Count = 1;
//    stagingDesc.Usage = D3D11_USAGE_STAGING;
//    stagingDesc.BindFlags = 0;
//    stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
//
//    ID3D11Texture2D* stagingTexture = nullptr;
//    Device->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
//
//    // 2. 복사할 영역 정의 (D3D11_BOX)
//    D3D11_BOX srcBox = {};
//    srcBox.left = static_cast<UINT>(pt.x);
//    srcBox.right = srcBox.left + 1; // 1픽셀 너비
//    srcBox.top = static_cast<UINT>(pt.y);
//    srcBox.bottom = srcBox.top + 1; // 1픽셀 높이
//    srcBox.front = 0;
//    srcBox.back = 1;
//    FVector4 UUIDColor{ 1, 1, 1, 1 }; 
//
//    if (stagingTexture == nullptr)
//        return DecodeUUIDColor(UUIDColor);
//
//    // 3. 특정 좌표만 복사
//   DeviceContext->CopySubresourceRegion(
//        stagingTexture, // 대상 텍스처
//        0,              // 대상 서브리소스
//        0, 0, 0,        // 대상 좌표 (x, y, z)
//        UUIDFrameBuffer, // 원본 텍스처
//        0,              // 원본 서브리소스
//        &srcBox         // 복사 영역
//    );
//
//    // 4. 데이터 매핑
//    D3D11_MAPPED_SUBRESOURCE mapped = {};
//    DeviceContext->Map(stagingTexture, 0, D3D11_MAP_READ, 0, &mapped);
//
//    // 5. 픽셀 데이터 추출 (1x1 텍스처이므로 offset = 0)
//    const BYTE* pixelData = static_cast<const BYTE*>(mapped.pData);
//
//    if (pixelData)
//    {
//        UUIDColor.x = static_cast<float>(pixelData[0]); // R
//        UUIDColor.y = static_cast<float>(pixelData[1]); // G
//        UUIDColor.z = static_cast<float>(pixelData[2]) ; // B
//        UUIDColor.a = static_cast<float>(pixelData[3]); // A
//    }
//
//    // 6. 매핑 해제 및 정리
//    DeviceContext->Unmap(stagingTexture, 0);
//    if (stagingTexture) stagingTexture->Release(); stagingTexture = nullptr;
//
//    return DecodeUUIDColor(UUIDColor);
//}
//
//uint32 FGraphicsDevice::DecodeUUIDColor(FVector4 UUIDColor) {
//    uint32_t W = static_cast<uint32_t>(UUIDColor.a) << 24;
//    uint32_t Z = static_cast<uint32_t>(UUIDColor.z) << 16;
//    uint32_t Y = static_cast<uint32_t>(UUIDColor.y) << 8;
//    uint32_t X = static_cast<uint32_t>(UUIDColor.x);
//
//    return W | Z | Y | X;
//}

bool FGraphicsDevice::CreateVertexShader(const FString& fileName, ID3DBlob** ppCode, ID3D11VertexShader** ppVertexShader) const
{
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef  _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif
    //shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

    ID3DBlob* errorBlob = nullptr;

    const std::filesystem::path current = std::filesystem::current_path();
    const std::filesystem::path fullpath = current / TEXT("Shaders") / *fileName;
    const std::wstring shaderFilePath = fullpath.wstring();

    const HRESULT hr = D3DCompileFromFile(shaderFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainVS", "vs_5_0", shaderFlags, 0, ppCode, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            // errorBlob에 저장된 메시지를 출력 (디버그 출력이나 콘솔 등)
            OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        abort();
    }

    if(Device == nullptr)
        return false;

    if (FAILED(Device->CreateVertexShader((*ppCode)->GetBufferPointer(), (*ppCode)->GetBufferSize(), nullptr, ppVertexShader)))
        return false;

    return true;
}

bool FGraphicsDevice::CreatePixelShader(const FString& fileName, ID3DBlob** ppCode, ID3D11PixelShader** ppPixelShader) const
{
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef  _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif
    shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;

    ID3DBlob* errorBlob = nullptr;

    const std::filesystem::path current = std::filesystem::current_path();
    const std::filesystem::path fullpath = current / TEXT("Shaders") / *fileName;
    const std::wstring shaderFilePath = fullpath.wstring();

    HRESULT hr = D3DCompileFromFile(shaderFilePath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "mainPS", "ps_5_0", shaderFlags, 0, ppCode, &errorBlob);

    if (FAILED(hr))
    {
        if (errorBlob)
        {
            // errorBlob에 저장된 메시지를 출력 (디버그 출력이나 콘솔 등)
            OutputDebugStringA(reinterpret_cast<const char*>(errorBlob->GetBufferPointer()));
            errorBlob->Release();
        }
        abort();
    }

    if (Device == nullptr)
        return false;

    if (FAILED(Device->CreatePixelShader((*ppCode)->GetBufferPointer(), (*ppCode)->GetBufferSize(), nullptr, ppPixelShader)))
        return false;

    return true;
}

TArray<FConstantBufferInfo> FGraphicsDevice::ExtractConstantBufferNames(ID3DBlob* shaderBlob)
{
    TArray<FConstantBufferInfo> CBInfos;

    // 쉐이더 리플렉션 인터페이스 생성
    ID3D11ShaderReflection* pReflector = nullptr;
    HRESULT hr = D3DReflect(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection,
                            reinterpret_cast<void**>(&pReflector));
    if (FAILED(hr) || pReflector == nullptr)
    {
        // 오류 처리: 빈 벡터 반환
        return CBInfos;
    }
    
    // 쉐이더 설명 가져오기
    D3D11_SHADER_DESC shaderDesc = {};
    hr = pReflector->GetDesc(&shaderDesc);
    assert(SUCCEEDED(hr));
    
    // 모든 상수 버퍼에 대해 이름을 추출
    for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
    {
        ID3D11ShaderReflectionConstantBuffer* pCB = pReflector->GetConstantBufferByIndex(i);
        if (pCB)
        {
            D3D11_SHADER_BUFFER_DESC cbDesc = {};
            hr = pCB->GetDesc(&cbDesc);
            if (SUCCEEDED(hr))
            {
                const FString CBName = cbDesc.Name;
                CBInfos.Add(FConstantBufferInfo(CBName, cbDesc.Size, i));
            }
        }
    }
    
    pReflector->Release();
    return CBInfos;
}

