#pragma once
#include <memory>
#include "Texture.h"
#include "Container/Map.h"
#include "Container/Set.h"
#include "Container/String.h"

class FRenderer;
class FGraphicsDevice;
class FResourceMgr
{

public:
    void Initialize(FRenderer* renderer, FGraphicsDevice* device);
    void Release(FRenderer* renderer);
    HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename);
    HRESULT LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);

    std::shared_ptr<FTexture> GetTexture(const FWString& name);

    TSet<FString> GetAllTextureKeys() const;

    void ReleaseSRVs();
    void CreateSRVs();
private:
    TMap<FWString, std::shared_ptr<FTexture>> textureMap;
};