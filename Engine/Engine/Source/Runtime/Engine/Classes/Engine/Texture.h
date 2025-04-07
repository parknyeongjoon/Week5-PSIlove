#pragma once
#include "D3D11RHI/GraphicDevice.h"
struct FTexture
{
    FTexture(const FWString& InName, const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& SRV,
             const Microsoft::WRL::ComPtr<ID3D11Texture2D>& Texture2D, const uint32 _width, const uint32 _height)
        : Name(InName),TextureSRV(SRV), Texture(Texture2D), width(_width), height(_height)
    {}
    ~FTexture()
    {
		
    }

    FWString Name;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture = nullptr;
    uint32 width;
    uint32 height;
};
