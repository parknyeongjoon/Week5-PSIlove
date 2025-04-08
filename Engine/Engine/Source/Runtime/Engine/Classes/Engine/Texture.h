#pragma once
#include "HAL/PlatformType.h"

#define _TCHAR_DEFINED
#include "d3d11.h"

struct FTexture
{
    FTexture(const FWString& InName, ID3D11ShaderResourceView* SRV,
             ID3D11Texture2D* Texture2D, const uint32 _width, const uint32 _height)
        : Name(InName),TextureSRV(SRV), Texture(Texture2D), width(_width), height(_height)
    {}
    
    ~FTexture()
    {
        TextureSRV->Release();
        TextureSRV = nullptr;
        
        Texture->Release();
        Texture = nullptr;
    }

    void ReleaseSRV() { TextureSRV->Release(); TextureSRV = nullptr; }
    ID3D11ShaderResourceView* CreateShaderResourceView();

    FWString Name;
    ID3D11ShaderResourceView* TextureSRV = nullptr;
    ID3D11Texture2D* Texture = nullptr;
    uint32 width;
    uint32 height;
};

