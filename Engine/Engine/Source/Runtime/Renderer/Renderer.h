#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Map.h"
#include "Container/Set.h"
#include "D3D11RHI/GraphicDevice.h"
// #include "D3D11RHI/GPUBuffer/FConstantBuffer.h"
// #include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "D3D11RHI/GPUBuffer/TestConstantDefine.h"

class FVIBuffers;
class ULightComponentBase;
class UBillboardComponent;
class UPrimitiveComponent;
class UGizmoBaseComponent;
class UStaticMeshComponent;
class ULevel;
class FShaderProgram;
class FEditorViewportClient;


class FRenderer 
{
public:
    FGraphicsDevice* Graphics;

    void AddOrSetVertexShader(const FString& InName, Microsoft::WRL::ComPtr<ID3D11VertexShader> InShader);
    void AddOrSetPixelShader(const FString& InName, Microsoft::WRL::ComPtr<ID3D11PixelShader> InShader);
    void AddOrSetInputLayout(const FString& InName, Microsoft::WRL::ComPtr<ID3D11InputLayout> InLayout);

    void AddOrSetVertexBuffer(const FString& InName,  Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, uint32 InStride);
    void AddOrSetIndexBuffer(const FString& InName,  Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, uint32 numIndices);

    void AddOrSetStructuredBuffer(const FString& InName,  Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer);
    void AddOrSetStructuredBufferShaderResourceView(const FString& InName,  Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> InShaderResourceView);
    
    std::shared_ptr<FVIBuffers> GetVIBuffer(const FString& InVIName) { return VIBuffers[InVIName]; }

    Microsoft::WRL::ComPtr<ID3D11Buffer> GetStructuredBuffer(const FString& InName) { return StructuredBuffers[InName].Key; }
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetStructuredBufferShaderResourceView(const FString& InName) { return StructuredBuffers[InName].Value; }

    Microsoft::WRL::ComPtr<ID3D11Buffer> GetConstantBuffer(const FString& InName) { return ConstantBuffers[InName]; }
private:
    TMap<FString, std::shared_ptr<FShaderProgram>> ShaderPrograms;
    TMap<FString, std::shared_ptr<FVIBuffers>> VIBuffers;

    TMap<FString, TMap<FShaderConstantKey, uint32>> ShaderConstantNames;
    TMap<FString, Microsoft::WRL::ComPtr<ID3D11Buffer>> ConstantBuffers;
    TMap<FString, TPair<Microsoft::WRL::ComPtr<ID3D11Buffer>, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>> StructuredBuffers;
public:
    FLightingBuffer lightingData;

    void Initialize(FGraphicsDevice* graphics);
    void CreateShader();
    void CreateTextureShader();
    void CreateFontShader();
    void CreateLineShader();    

    void PrepareShader(const FString& InShaderName) const;

    void BindConstantBuffers(const FString& InShaderName) const;
    
    // //Render
    // void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) const;
    // void RenderPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices) const;
    // void RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex) const;
    //
    // void RenderTexturedModelPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV, ID3D11SamplerState* InSamplerState) const;
    
    void ChangeViewMode(EViewModeIndex evi);
    
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateImmutableVertexBuffer(const TArray<T>& vertices) const;
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateImmutableVertexBuffer(T* vertices, uint32 arraySize) const;
    
    template <typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStructuredBuffer(uint32 numElements) const;
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStaticVertexBuffer(const TArray<T>& vertices) const;
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStaticVertexBuffer(T* vertices, uint32 arraySize) const;
    
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicVertexBuffer(const TArray<T>& vertices) const;
    template<typename T>
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicVertexBuffer(T* vertices, uint32 arraySize) const;
    
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(const uint32* indices, uint32 indicesSize) const;
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateIndexBuffer(const TArray<uint32>& indices) const;
    
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(uint32 InSize, const void* InData = nullptr) const;

    template<typename T>
    void UpdateConstant(Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, const T* InData = nullptr);

    template <typename T>
    void UpdateStructuredBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer, const TArray<T>& Data) const;

    // update
    // void UpdateLightBuffer() const;
    // void UpdateConstant(const FMatrix& MVP, const FMatrix& NormalMatrix, FVector4 UUIDColor, bool IsSelected) const;
    // void UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const;
    // void UpdateLitUnlitConstant(int isLit) const;
    // void UpdateSubMeshConstant(bool isSelected) const;
    // void UpdateTextureConstant(float UOffset, float VOffset);

public:    
    //void PrepareTextureShader() const;
    
    // void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer,
    //                             ID3D11Buffer* pIndexBuffer, UINT numIndices,
    //                             ID3D11ShaderResourceView* _TextureSRV,
    //                             ID3D11SamplerState* _SamplerState) const;
    // void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    //     ID3D11ShaderResourceView* _TextureSRV,
    //     ID3D11SamplerState* _SamplerState) const;

    // void UpdateSubUVConstant(float _indexU, float _indexV) const;
    // void PrepareSubUVConstant() const;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateBufferSRV(const Microsoft::WRL::ComPtr<ID3D11Buffer>& pBuffer, UINT numElements) const;

public: // line shader
    //void PrepareLineShader() const;
    //void ReleaseLineShader() const;
    //void RenderBatch(const FGridParametersData& gridParam, Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount) const;
    void PrepareRender(const ULevel* Level);
    void UpdateGridConstantBuffer(const FGridParametersData& gridParams) const;
    void UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const;

    // void UpdateBoundingBoxBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes) const;
    // void UpdateOBBBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer> pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes) const;
    // void UpdateConesBuffer(Microsoft::WRL::ComPtr<ID3D11Buffer>pConeBuffer, const TArray<FCone>& Cones) const;

    //Render Pass Demo
    void ClearRenderArr();
    void Render(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderStaticMeshes(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderGizmos(const ULevel* Level, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderLight(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderBillboards(ULevel* Level,std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderTexts(ULevel* Level,std::shared_ptr<FEditorViewportClient> ActiveViewport);
    
private:
    TArray<UStaticMeshComponent*> StaticMeshObjs;
    TArray<UGizmoBaseComponent*> GizmoObjs;
    TArray<UPrimitiveComponent*> TextObjs;
    TArray<UBillboardComponent*> BillboardObjs;
    TArray<ULightComponentBase*> LightObjs;
};

template <typename T>
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateImmutableVertexBuffer(const TArray<T>& vertices) const
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
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateImmutableVertexBuffer(T* vertices, uint32 arraySize) const
{
    TArray<T> verticeArray;
    verticeArray.AppendArray(vertices, arraySize);

    return CreateImmutableVertexBuffer(verticeArray);
}

template <typename T>
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateDynamicVertexBuffer(const TArray<T>& vertices) const
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
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateDynamicVertexBuffer(T* vertices, uint32 arraySize) const
{
    TArray<T> verticeArray;
    verticeArray.AppendArray(vertices, arraySize);

    return CreateDynamicVertexBuffer(verticeArray);
}

template <typename T>
void FRenderer::UpdateConstant(Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, const T* InData)
{
    D3D11_MAPPED_SUBRESOURCE sub = {};
    Graphics->DeviceContext->Map(InBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &sub);
    memcpy(sub.pData, InData, sizeof(T));
    Graphics->DeviceContext->Unmap(InBuffer.Get(), 0);
}

template <typename T>
void FRenderer::UpdateStructuredBuffer(const Microsoft::WRL::ComPtr<ID3D11Buffer> pBuffer, const TArray<T>& Data) const
{
    if (!pBuffer)
        return;

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    const HRESULT hr = Graphics->DeviceContext->Map(pBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
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

    Graphics->DeviceContext->Unmap(pBuffer.Get(), 0);
}

template <typename T>
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateStructuredBuffer(const uint32 numElements) const
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
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateStaticVertexBuffer(const TArray<T>& vertices) const
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
Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateStaticVertexBuffer(T* vertices, uint32 arraySize) const
{
    TArray<T> verticeArray;
    verticeArray.AppendArray(vertices, arraySize);

    return CreateStaticVertexBuffer(verticeArray);
}


