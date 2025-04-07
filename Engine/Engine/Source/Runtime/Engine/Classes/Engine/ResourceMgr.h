#pragma once
#include <memory>
#include "Texture.h"
#include "Container/Map.h"
#include "Container/Set.h"
#include "Container/String.h"

#define _TCHAR_DEFINED
#include <wrl.h>

class FRenderer;
class FGraphicsDevice;
class FResourceMgr
{

public:
    void Initialize(FRenderer* renderer, FGraphicsDevice* device);
    void Release(FRenderer* renderer);
    HRESULT LoadTextureFromFile(Microsoft::WRL::ComPtr<ID3D11Device> device, const wchar_t* filename);
    HRESULT LoadTextureFromDDS(Microsoft::WRL::ComPtr<ID3D11Device> device, Microsoft::WRL::ComPtr<ID3D11DeviceContext> context, const wchar_t* filename);

    std::shared_ptr<FTexture> GetTexture(const FWString& name);

    TSet<FString> GetAllTextureKeys() const;
    
private:
    TMap<FWString, std::shared_ptr<FTexture>> textureMap;
};