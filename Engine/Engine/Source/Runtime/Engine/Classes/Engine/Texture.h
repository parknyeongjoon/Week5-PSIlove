#pragma once
#include "D3D11RHI/GraphicDevice.h"
struct FTexture
{
    FTexture(const FWString& InName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SRV, Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture2D, Microsoft::WRL::ComPtr<ID3D11SamplerState> Sampler, uint32 _width, uint32 _height)
        : Name(InName),TextureSRV(SRV), Texture(Texture2D), SamplerState(Sampler), width(_width), height(_height)
    {}
    ~FTexture()
    {
		
    }
    void Release() {
        if (TextureSRV) { TextureSRV->Release(); TextureSRV = nullptr; }
        if (Texture) { Texture->Release(); Texture = nullptr; }
        if (SamplerState) { SamplerState->Release(); SamplerState = nullptr; }
    }
    FWString Name;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> TextureSRV = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture = nullptr;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> SamplerState = nullptr;
    uint32 width;
    uint32 height;
};
