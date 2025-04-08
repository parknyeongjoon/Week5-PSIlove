#include "ResourceMgr.h"
#include <fstream>
#include <wincodec.h>
#include <ranges>
#include "Define.h"
#include "Components/SkySphereComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "DirectXTK/Include/DDSTextureLoader.h"
#include "DirectXTK/Include/WICTextureLoader.h"

void FResourceMgr::Initialize(FRenderer* renderer, FGraphicsDevice* device)
{
    LoadTextureFromFile(renderer->Graphics->Device, L"Editor/Icon/Pawn_64x.png");
    LoadTextureFromFile(renderer->Graphics->Device, L"Editor/Icon/PointLight_64x.png");
    LoadTextureFromFile(renderer->Graphics->Device, L"Editor/Icon/SpotLight_64x.png");
    LoadTextureFromFile(renderer->Graphics->Device, L"Editor/Icon/S_Actor.png");
}

void FResourceMgr::Release(FRenderer* renderer)
{
    textureMap.Empty();
}

struct PairHash
{
	template <typename T1, typename T2>
	std::size_t operator()(const std::pair<T1, T2>& pair) const {
		return std::hash<T1>()(pair.first) ^ (std::hash<T2>()(pair.second) << 1);
	}
};

struct TupleHash
{
	template <typename T1, typename T2, typename T3>
	std::size_t operator()(const std::tuple<T1, T2, T3>& tuple) const {
		const std::size_t h1 = std::hash<T1>()(std::get<0>(tuple));
		const std::size_t h2 = std::hash<T2>()(std::get<1>(tuple));
		const std::size_t h3 = std::hash<T3>()(std::get<2>(tuple));

		return h1 ^ (h2 << 1) ^ (h3 << 2);  // 해시 값 섞기
	}
};

std::shared_ptr<FTexture> FResourceMgr::GetTexture(const FWString& name)
{
    auto* TempValue = textureMap.Find(name);
    if (!TempValue)
    {
        LoadTextureFromFile(FEngineLoop::graphicDevice.Device, name.c_str());
        return *textureMap.Find(name);
    }
    return TempValue ? *TempValue : nullptr;
}

TSet<FString> FResourceMgr::GetAllTextureKeys() const
{
    TSet<FString> Keys;
    for (const auto& Pair : textureMap)
    {
        const wchar_t* wstr = Pair.Key.c_str();
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
        char* str = new char[size_needed];
        WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, size_needed, nullptr, nullptr);
        Keys.Add(str);
    }
    return Keys;
}

void FResourceMgr::ReleaseSRVs()
{
    for (auto item : textureMap)
    {
        item.Value->ReleaseSRV();
    }
}

void FResourceMgr::CreateSRVs()
{
    for (auto item : textureMap)
    {
        
    }
    
}

HRESULT FResourceMgr::LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename)
{
	IWICImagingFactory* wicFactory = nullptr;
	IWICBitmapDecoder* decoder = nullptr;
	IWICBitmapFrameDecode* frame = nullptr;
	IWICFormatConverter* converter = nullptr;

	// WIC 팩토리 생성
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr)) return hr;

	hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&wicFactory));
	if (FAILED(hr)) return hr;

	// 이미지 파일 디코딩
	hr = wicFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
	if (FAILED(hr)) return hr;

	hr = decoder->GetFrame(0, &frame);
	if (FAILED(hr)) return hr;

	// WIC 포맷 변환기 생성 (픽셀 포맷 변환)
	hr = wicFactory->CreateFormatConverter(&converter);
	if (FAILED(hr)) return hr;

	hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeCustom);
	if (FAILED(hr)) return hr;

	// 이미지 크기 가져오기
	UINT width, height;
	frame->GetSize(&width, &height);
	
	// 픽셀 데이터 로드
	BYTE* imageData = new BYTE[width * height * 4];
	hr = converter->CopyPixels(nullptr, width * 4, width * height * 4, imageData);
	if (FAILED(hr)) {
		delete[] imageData;
		return hr;
	}

	// DirectX 11 텍스처 생성
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = imageData;
	initData.SysMemPitch = width * 4;
	ID3D11Texture2D* Texture2D;
	hr = device->CreateTexture2D(&textureDesc, &initData, &Texture2D);
    if (FAILED(hr)) assert(false || TEXT("CreateTexture Failed"));
	delete[] imageData;
	if (FAILED(hr)) return hr;

	// Shader Resource View 생성
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	ID3D11ShaderResourceView* TextureSRV;
	hr = device->CreateShaderResourceView(Texture2D, &srvDesc, &TextureSRV);
    if (FAILED(hr)) assert(false || TEXT("CreateShaderResourceView Failed"));

	// 리소스 해제
	wicFactory->Release();
	decoder->Release();
	frame->Release();
	converter->Release();
    
	textureMap[filename] = std::make_shared<FTexture>(filename, TextureSRV, Texture2D, width, height);

	Console::GetInstance().AddLog(LogLevel::Warning, "Texture File Load Successs");
	return hr;
}

HRESULT FResourceMgr::LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename)
{

	ID3D11Resource* texture = nullptr;
	ID3D11ShaderResourceView* textureView = nullptr;

	HRESULT hr = DirectX::CreateDDSTextureFromFile(
		device, context,
		filename,
		&texture,
		&textureView
	);
	if (FAILED(hr) || texture == nullptr) abort();

#pragma region WidthHeight

	ID3D11Texture2D* texture2D = nullptr;
	hr = texture->QueryInterface(__uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&texture2D));
	if (FAILED(hr) || texture2D == nullptr)
	{
		std::wcerr << L"Failed to query ID3D11Texture2D interface!" << std::endl;
		abort();
		return hr;
	}

	// 🔹 텍스처 크기 가져오기
	D3D11_TEXTURE2D_DESC texDesc;
	texture2D->GetDesc(&texDesc);
	uint32 width = texDesc.Width;
	uint32 height = texDesc.Height;

#pragma endregion WidthHeight
	FWString name = FWString(filename);

	textureMap[name] = std::make_shared<FTexture>(name, textureView, texture2D, width, height);

	Console::GetInstance().AddLog(LogLevel::Warning, "Texture File Load Successs");

	return hr;
}
