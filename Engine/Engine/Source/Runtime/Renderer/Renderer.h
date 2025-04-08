#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Map.h"
#include "Container/Set.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/GPUBuffer/TestConstantDefine.h"

class FEditorViewportClient;
class ULevel;
class BaseRenderPass;
class FShaderProgram;
class FVIBuffers;

class FRenderer 
{
public:
    FGraphicsDevice* Graphics;

    void AddOrSetVertexShader(const FString& InName, ID3D11VertexShader* InShader);
    void AddOrSetPixelShader(const FString& InName, ID3D11PixelShader* InShader);
    void AddOrSetInputLayout(const FString& InName, ID3D11InputLayout* InLayout);

    void AddOrSetVertexBuffer(const FString& InName, ID3D11Buffer* InBuffer, uint32 InStride, D3D11_PRIMITIVE_TOPOLOGY InTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    void AddOrSetIndexBuffer(const FString& InName, ID3D11Buffer* InBuffer, uint32 numIndices);

    void AddOrSetStructuredBuffer(const FString& InName, ID3D11Buffer* InBuffer);
    void AddOrSetStructuredBufferShaderResourceView(const FString& InName, ID3D11ShaderResourceView* InShaderResourceView);
    
    std::shared_ptr<FVIBuffers> GetVIBuffer(const FString& InVIName) { return VIBuffers[InVIName]; }

    ID3D11Buffer* GetStructuredBuffer(const FString& InName) { return StructuredBuffers[InName].Key; }
    ID3D11ShaderResourceView* GetStructuredBufferShaderResourceView(const FString& InName) { return StructuredBuffers[InName].Value; }

    ID3D11Buffer* GetConstantBuffer(const FString& InName) { return ConstantBuffers[InName]; }

    ID3D11SamplerState* GetSamplerState(ESamplerType InType) const { return SamplerStates[static_cast<uint32>(InType)]; }
    ID3D11RasterizerState* GetRasterizerState(ERasterizerState InState) const { return RasterizerStates[static_cast<uint32>(InState)]; }
    ID3D11BlendState* GetBlendState(EBlendState InState) const { return BlendStates[static_cast<uint32>(InState)]; }
    ID3D11DepthStencilState* GetDepthStencilState(EDepthStencilState InState) const { return DepthStencilStates[static_cast<uint32>(InState)]; }

    ERasterizerState GetCurrentRasterizerState() const {  return CurrentRasterizerState; }
    void SetCurrentRasterizerState(const ERasterizerState InState) { CurrentRasterizerState = InState; }
private:
    TMap<FString, std::shared_ptr<FShaderProgram>> ShaderPrograms;
    TMap<FString, std::shared_ptr<FVIBuffers>> VIBuffers;

    TMap<FString, TMap<FShaderConstantKey, uint32>> ShaderConstantNames;
    TMap<FString, ID3D11Buffer*> ConstantBuffers;
    TMap<FString, TPair<ID3D11Buffer*, ID3D11ShaderResourceView*>> StructuredBuffers;

    ID3D11SamplerState* SamplerStates[static_cast<uint32>(ESamplerType::End)] = {};
    
    ID3D11RasterizerState* RasterizerStates[static_cast<uint32>(ERasterizerState::End)] = {};
    ERasterizerState CurrentRasterizerState = ERasterizerState::SolidBack;
    
    ID3D11BlendState* BlendStates[static_cast<uint32>(EBlendState::End)] = {};
    
    ID3D11DepthStencilState* DepthStencilStates[static_cast<uint32>(EDepthStencilState::End)] = {};

public:

    void Initialize(FGraphicsDevice* graphics);
    void CreateStaticMeshShader();
    void CreateTextureShader();
    void CreateFontShader();
    void CreateLineShader();
    void LoadStates();

    void Release();
    void ReleaseShaders();
    void ReleaseConstantBuffers();
    void ReleaseStates();

    void ReBindSamplers() const;

    void PrepareShader(const FString& InShaderName) const;

    void BindConstantBuffers(const FString& InShaderName) const;
    
    void ChangeViewMode(EViewModeIndex evi);
    
    template<typename T>
    ID3D11Buffer* CreateImmutableVertexBuffer(const TArray<T>& vertices) const;
    template<typename T>
    ID3D11Buffer* CreateImmutableVertexBuffer(T* vertices, uint32 arraySize) const;
    
    template <typename T>
    ID3D11Buffer* CreateStructuredBuffer(uint32 numElements) const;
    template<typename T>
    ID3D11Buffer* CreateStaticVertexBuffer(const TArray<T>& vertices) const;
    template<typename T>
    ID3D11Buffer* CreateStaticVertexBuffer(T* vertices, uint32 arraySize) const;
    
    template<typename T>
    ID3D11Buffer* CreateDynamicVertexBuffer(const TArray<T>& vertices) const;
    template<typename T>
    ID3D11Buffer* CreateDynamicVertexBuffer(T* vertices, uint32 arraySize) const;
    
    ID3D11Buffer* CreateIndexBuffer(const uint32* indices, uint32 indicesSize) const;
    ID3D11Buffer* CreateIndexBuffer(const TArray<uint32>& indices) const;
    
    ID3D11Buffer* CreateConstantBuffer(uint32 InSize, const void* InData = nullptr) const;

    template<typename T>
    void UpdateConstant(ID3D11Buffer* InBuffer, const T* InData = nullptr);
    
    template <typename T>
    void UpdateStructuredBuffer(ID3D11Buffer* pBuffer, const TArray<T>& Data) const;

public:    
    ID3D11ShaderResourceView* CreateBufferSRV(ID3D11Buffer* pBuffer, UINT numElements) const;

    //void PrepareRender(const ULevel* Level);
    void AddRenderObjectsToRenderPass(const ULevel* InLevel);
    
    //Render Pass Demo
    void Render(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
private:
    TArray<std::shared_ptr<BaseRenderPass>> RenderPasses;
};

template <typename T>
ID3D11Buffer* FRenderer::CreateImmutableVertexBuffer(const TArray<T>& vertices) const
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = sizeof(T) * vertices.Num();
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // 한 번 생성 후 업데이트하지 않음
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {};
    vertexbufferSRD.pSysMem = vertices.GetData();

    ID3D11Buffer* vertexBuffer = nullptr;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation failed");
        return nullptr;
    }
    return vertexBuffer;
}

template <typename T>
ID3D11Buffer* FRenderer::CreateImmutableVertexBuffer(T* vertices, uint32 arraySize) const
{
    TArray<T> verticeArray;
    verticeArray.AppendArray(vertices, arraySize);

    return CreateImmutableVertexBuffer(verticeArray);
}

template <typename T>
ID3D11Buffer* FRenderer::CreateDynamicVertexBuffer(const TArray<T>& vertices) const
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = sizeof(T) * vertices.Num();
    vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // 업데이트 가능
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {};
    vertexbufferSRD.pSysMem = vertices.GetData();

    ID3D11Buffer* vertexBuffer = nullptr;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation failed");
        return nullptr;
    }
    return vertexBuffer;
}

template <typename T>
ID3D11Buffer* FRenderer::CreateDynamicVertexBuffer(T* vertices, uint32 arraySize) const
{
    TArray<T> verticeArray;
    verticeArray.AppendArray(vertices, arraySize);

    return CreateDynamicVertexBuffer(verticeArray);
}

template <typename T>
void FRenderer::UpdateConstant(ID3D11Buffer* InBuffer, const T* InData)
{
    D3D11_MAPPED_SUBRESOURCE sub = {};
    const HRESULT hr = Graphics->DeviceContext->Map(InBuffer, 0,D3D11_MAP_WRITE_DISCARD,0, &sub);
    if (FAILED(hr))
    {
        assert(TEXT("Map failed"));
    }
    memcpy(sub.pData, InData, sizeof(T));
    Graphics->DeviceContext->Unmap(InBuffer, 0);
}

template <typename T>
void FRenderer::UpdateStructuredBuffer(ID3D11Buffer* pBuffer, const TArray<T>& Data) const
{
    if (!pBuffer)
        return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    const HRESULT hr = Graphics->DeviceContext->Map(pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        // 오류 처리 (필요 시 로그 출력)
        return;
    }

    auto pData = reinterpret_cast<T*>(mappedResource.pData);
    for (int i = 0; i < Data.Num(); ++i)
    {
        pData[i] = Data[i];
    }

    Graphics->DeviceContext->Unmap(pBuffer, 0);
}

template <typename T>
ID3D11Buffer* FRenderer::CreateStructuredBuffer(const uint32 numElements) const
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // CPU가 데이터를 업데이트할 수 있도록 설정
    bufferDesc.ByteWidth = sizeof(T) * numElements;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(T);

    ID3D11Buffer* buffer = nullptr;
    HRESULT hr = Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &buffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "Structured Buffer Creation failed");
        return nullptr;
    }
    return buffer;
}

template <typename T>
ID3D11Buffer* FRenderer::CreateStaticVertexBuffer(const TArray<T>& vertices) const
{
    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;  // 정적 버퍼: 한 번 생성 후 업데이트하지 않음
    vbDesc.ByteWidth =  sizeof(T) * vertices.Num(); // 정점 데이터 개수에 따라 총 바이트 크기를 계산합니다.
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;           // CPU에서 직접 접근하지 않음

    D3D11_SUBRESOURCE_DATA vbInitData = {};
    vbInitData.pSysMem = vertices.GetData();

    ID3D11Buffer* pVertexBuffer = nullptr;
    HRESULT hr = Graphics->Device->CreateBuffer(&vbDesc, &vbInitData, &pVertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "Static Vertex Buffer Creation failed");
        return nullptr;
    }
    return pVertexBuffer;
}

template <typename T>
ID3D11Buffer* FRenderer::CreateStaticVertexBuffer(T* vertices, uint32 arraySize) const
{
    TArray<T> verticeArray;
    verticeArray.AppendArray(vertices, arraySize);

    return CreateStaticVertexBuffer(verticeArray);
}


