#include "Renderer.h"
#include <d3dcompiler.h>

#include "Level.h"
#include "Actors/Player.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ParticleSubUVComp.h"
#include "Components/TextBillboardComponent.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EngineLoop.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/Casts.h"
#include "UObject/Object.h"
#include "PropertyEditor/ShowFlags.h"
#include "UObject/UObjectIterator.h"
#include "Components/SkySphereComponent.h"
#include "Components/TextRenderComponent.h"
#include "D3D11RHI/FShaderProgram.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "D3D11RHI/GPUBuffer/TestConstantDefine.h"
#include "ImGUI/imgui_internal.h"

void FRenderer::AddOrSetVertexShader(const FString& InName, Microsoft::WRL::ComPtr<ID3D11VertexShader> InShader)
{
    if (ShaderPrograms.Contains(InName) == false)
    {
        ShaderPrograms[InName] = std::make_shared<FShaderProgram>();
    }
    ShaderPrograms[InName]->SetVertexShader(InShader);
}

void FRenderer::AddOrSetPixelShader(const FString& InName, Microsoft::WRL::ComPtr<ID3D11PixelShader> InShader)
{
    if (ShaderPrograms.Contains(InName) == false)
    {
        ShaderPrograms[InName] = std::make_shared<FShaderProgram>();
    }
    ShaderPrograms[InName]->SetPixelShader(InShader);
}

void FRenderer::AddOrSetInputLayout(const FString& InName, Microsoft::WRL::ComPtr<ID3D11InputLayout> InLayout)
{
    if (ShaderPrograms.Contains(InName) == false)
    {
        ShaderPrograms[InName] = std::make_shared<FShaderProgram>();
    }
    ShaderPrograms[InName]->SetInputLayout(InLayout);
}

void FRenderer::AddOrSetVertexBuffer(const FString& InName, Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, const uint32 InStride)
{
    if (VIBuffers.Contains(InName) == false)
    {
        VIBuffers[InName] = std::make_shared<FVIBuffers>();
    }
    VIBuffers[InName]->SetVertexBuffer(InBuffer, InStride);    
}

void FRenderer::AddOrSetIndexBuffer(const FString& InName, Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer, const uint32 numIndices)
{
    if (VIBuffers.Contains(InName) == false)
    {
        VIBuffers[InName] = std::make_shared<FVIBuffers>();
    }
    VIBuffers[InName]->SetIndexBuffer(InBuffer, numIndices);
}

void FRenderer::AddOrSetStructuredBuffer(const FString& InName, Microsoft::WRL::ComPtr<ID3D11Buffer> InBuffer)
{
    if (StructuredBuffers.Contains(InName) == false)
    {
        StructuredBuffers[InName] = TPair<Microsoft::WRL::ComPtr<ID3D11Buffer>, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>();
    }
    StructuredBuffers[InName].Key = InBuffer;
}

void FRenderer::AddOrSetStructuredBufferShaderResourceView(const FString& InName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> InShaderResourceView)
{
    if (StructuredBuffers.Contains(InName) == false)
    {
        StructuredBuffers[InName] = TPair<Microsoft::WRL::ComPtr<ID3D11Buffer>, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>();
    }
    StructuredBuffers[InName].Value = InShaderResourceView;
}

void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();
    CreateTextureShader();
    CreateFontShader();
    CreateLineShader();
}


void FRenderer::CreateShader()
{
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderCSO;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderCSO;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
    
    Graphics->CreateVertexShader(TEXT("StaticMeshVertexShader.hlsl"), VertexShaderCSO.GetAddressOf(), VertexShader.GetAddressOf());
    Graphics->CreatePixelShader(TEXT("StaticMeshPixelShader.hlsl"), PixelShaderCSO.GetAddressOf(), PixelShader.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), InputLayout.GetAddressOf()
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO.Get());
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO.Get());

    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : VertexStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PixelStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    ShaderPrograms.Add(TEXT("StaticMesh"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout, sizeof(FVertexSimple)));
    ShaderConstantNames.Add(TEXT("StaticMesh"), ShaderStageToCB);
}

void FRenderer::CreateTextureShader()
{
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderCSO;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderCSO;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
    
    Graphics->CreateVertexShader(TEXT("TextureVertexShader.hlsl"), VertexShaderCSO.GetAddressOf(), VertexShader.GetAddressOf());
    Graphics->CreatePixelShader(TEXT("TexturePixelShader.hlsl"), PixelShaderCSO.GetAddressOf(), PixelShader.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), InputLayout.GetAddressOf()
    );
    
    const TArray<FConstantBufferInfo> VertexTextureMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO.Get());
    const TArray<FConstantBufferInfo> PixelTextureMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO.Get());

    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : VertexTextureMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PixelTextureMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    ShaderPrograms.Add(TEXT("Texture"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout, sizeof(FVertexTexture)));
    ShaderConstantNames.Add(TEXT("Texture"), ShaderStageToCB);
}

void FRenderer::CreateFontShader()
{
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderCSO;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderCSO;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
    
    
    Graphics->CreateVertexShader(TEXT("FontVertexShader.hlsl"), VertexShaderCSO.GetAddressOf(), VertexShader.GetAddressOf());

    Graphics->CreatePixelShader(TEXT("FontPixelShader.hlsl"), PixelShaderCSO.GetAddressOf(), PixelShader.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), InputLayout.GetAddressOf()
    );
    
    const TArray<FConstantBufferInfo> VertexFontMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO.Get());
    const TArray<FConstantBufferInfo> PixelFontMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO.Get());

    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : VertexFontMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PixelFontMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    ShaderPrograms.Add(TEXT("Font"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout, sizeof(FVertexTexture)));
    ShaderConstantNames.Add(TEXT("Font"), ShaderStageToCB);
    
    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::CreateLineShader()
{
    Microsoft::WRL::ComPtr<ID3DBlob> VertexShaderCSO;
    Microsoft::WRL::ComPtr<ID3DBlob> PixelShaderCSO;

    Microsoft::WRL::ComPtr<ID3D11VertexShader> VertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> PixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> InputLayout;
    
    Graphics->CreateVertexShader(TEXT("ShaderLineVertexShader.hlsl"),VertexShaderCSO.GetAddressOf(), VertexShader.GetAddressOf());
    Graphics->CreatePixelShader(TEXT("ShaderLinePixelShader.hlsl"), PixelShaderCSO.GetAddressOf(), PixelShader.GetAddressOf());

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        // 정점 ID: 32비트 부호 없는 정수, 입력 슬로트 0, Per-Vertex 데이터
        { "SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    
        // 인스턴스 ID: 32비트 부호 없는 정수, 입력 슬로트 1, Per-Instance 데이터
        { "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    
    Graphics->Device->CreateInputLayout(
    layoutDesc, ARRAYSIZE(layoutDesc), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), InputLayout.GetAddressOf()
    );
    
    const TArray<FConstantBufferInfo> VertexLineConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO.Get());
    const TArray<FConstantBufferInfo> PixelLineConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO.Get());

    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : VertexLineConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PixelLineConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }
    
    ShaderPrograms.Add(TEXT("Line"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout, sizeof(FSimpleVertex)));
    ShaderConstantNames.Add(TEXT("Line"), ShaderStageToCB);
}

void FRenderer::PrepareShader(const FString& InShaderName) const
{
    ShaderPrograms[InShaderName]->Bind(Graphics->DeviceContext.Get());

    BindConstantBuffers(InShaderName);
}

void FRenderer::BindConstantBuffers(const FString& InShaderName) const
{
    TMap<FShaderConstantKey, uint32> curShaderBindedConstant = ShaderConstantNames[InShaderName];
    for (const auto item : curShaderBindedConstant)
    {
        auto curConstantBuffer = ConstantBuffers[item.Key.ConstantName];
        if (item.Key.ShaderType == EShaderStage::VS)
        {
            Graphics->DeviceContext->VSSetConstantBuffers(item.Value, 1, &curConstantBuffer);
        }
        else if (item.Key.ShaderType == EShaderStage::PS)
        {
            Graphics->DeviceContext->PSSetConstantBuffers(item.Value, 1, &curConstantBuffer);
        }
    }
}

void FRenderer::ChangeViewMode(const EViewModeIndex evi)
{
    FFlagConstants flag;
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        flag.IsLit = true;
        UpdateConstant<FFlagConstants>(ConstantBuffers[TEXT("FFlagConstants")], &flag);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        flag.IsLit = false;
        UpdateConstant<FFlagConstants>(ConstantBuffers[TEXT("FFlagConstants")], &flag);
        break;
    }
}

Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateIndexBuffer(const uint32* indices, const uint32 indicesSize) const
{
    TArray<uint32> indicesToCopy;
    indicesToCopy.AppendArray(indices, indicesSize);

    return CreateIndexBuffer(indicesToCopy);
}

Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateIndexBuffer(const TArray<uint32>& indices) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.ByteWidth = sizeof(uint32) * indices.Num();

    D3D11_SUBRESOURCE_DATA indexbufferSRD;
    indexbufferSRD.pSysMem = indices.GetData();

   Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

Microsoft::WRL::ComPtr<ID3D11Buffer> FRenderer::CreateConstantBuffer(const uint32 InSize, const void* InData) const
{
    D3D11_BUFFER_DESC constantBufferDesc = {};   
    constantBufferDesc.ByteWidth = InSize;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sub = {};
    sub.pSysMem = InData;
    
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;

    HRESULT hr;
    if (InData == nullptr)
        hr = Graphics->Device->CreateBuffer(&constantBufferDesc, nullptr, constantBuffer.GetAddressOf());
    else
        hr = Graphics->Device->CreateBuffer(&constantBufferDesc, &sub, constantBuffer.GetAddressOf());

    if (FAILED(hr))
        assert(NULL/*"Create constant buffer failed!"*/);

    return constantBuffer;
}

// void FRenderer::RenderTexturePrimitive(
//     ID3D11Buffer* pVertexBuffer, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
//     ID3D11SamplerState* _SamplerState
// ) const
// {
//     if (!_TextureSRV || !_SamplerState)
//     {
//         Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
//     }
//     if (numIndices <= 0)
//     {
//         Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
//     }
//     UINT offset = 0;
//     Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
//     Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//     Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//     Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
//     Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
//
//     Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
// }

// //��Ʈ ��ġ������
// void FRenderer:: mRenderTextPrimitive(
//     ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
// ) const
// {
//     if (!_TextureSRV || !_SamplerState)
//     {
//         Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
//     }
//     UINT offset = 0;
//     Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
//
//     // �Է� ���̾ƿ� �� �⺻ ����
//     Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//     Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
//     Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
//     
//     ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
//     Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);
//
//     // ��ο� ȣ�� (6���� �ε��� ���)
//     Graphics->DeviceContext->Draw(numVertices, 0);
// }

Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> FRenderer::CreateBufferSRV(const Microsoft::WRL::ComPtr<ID3D11Buffer>& pBuffer, const UINT numElements) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 구조화된 버퍼의 경우 형식은 UNKNOWN으로 지정
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numElements;

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pSRV = nullptr;
    const HRESULT hr = Graphics->Device->CreateShaderResourceView(pBuffer.Get(), &srvDesc, pSRV.GetAddressOf());
    if (FAILED(hr))
    {
        // 오류 처리 (필요에 따라 로그 출력 등)
        assert(false && "CreateStructuredBufferShaderResourceView failed");
        return nullptr;
    }
    return pSRV;
}

void FRenderer::UpdateGridConstantBuffer(const FGridParametersData& gridParams) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(ConstantBuffers[TEXT("FGridParametersData")].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &gridParams, sizeof(FGridParametersData));
        Graphics->DeviceContext->Unmap(ConstantBuffers[TEXT("FGridParametersData")].Get(), 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "gridParams ���� ����");
    }
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(ConstantBuffers[TEXT("FPrimitiveCounts")].Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    pData->BoundingBoxCount = numBoundingBoxes;
    pData->ConeCount = numCones;
    Graphics->DeviceContext->Unmap(ConstantBuffers[TEXT("FPrimitiveCounts")].Get(), 0);
}

// void FRenderer::RenderBatch(
//     const FGridParametersData& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount
// ) const
// {
//     UINT stride = sizeof(FSimpleVertex);
//     UINT offset = 0;
//     Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
//     Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
//
//     UINT vertexCountPerInstance = 2;
//     UINT instanceCount = gridParam.GridCount + 3 + (boundingBoxCount * 12) + (coneCount * (2 * coneSegmentCount)) + (12 * obbCount);
//     Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
//     Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
// }

void FRenderer::PrepareRender(const ULevel* Level)
{
    TArray<USceneComponent*> Ss;
    for (const auto& A : Level->GetActors())
    {
        Ss.Add(A->GetRootComponent());
        TArray<USceneComponent*> temp;
        A->GetRootComponent()->GetChildrenComponents(temp);
        Ss + temp;
    }

    for (const USceneComponent* iter : TObjectRange<USceneComponent>())
    {
        if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
        {
            GizmoObjs.Add(pGizmoComp);
        }
        if (UTextRenderComponent* TextRenderComp = Cast<UTextRenderComponent>(iter))
        {
            TextObjs.Add(TextRenderComp);
        }
        if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
        {
            LightObjs.Add(pLightComp);
        }
    }
    
    for (const auto iter : Ss)
    {
        if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
        {
            if (!Cast<UGizmoBaseComponent>(iter))
                StaticMeshObjs.Add(pStaticMeshComp);
        }

        if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
        {
            if (UTextBillboardComponent* TextBillboardComp = Cast<UTextBillboardComponent>(iter))
            {
                TextObjs.Add(TextBillboardComp);
            }
            else
            {
                BillboardObjs.Add(pBillboardComp);
            }
        }

    }
}

void FRenderer::ClearRenderArr()
{
    StaticMeshObjs.Empty();
    GizmoObjs.Empty();
    TextObjs.Empty();
    LightObjs.Empty();
}

void FRenderer::Render(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    ChangeViewMode(ActiveViewport->GetViewMode());
    //UpdateLightBuffer();
    UPrimitiveBatch::GetInstance().RenderBatch(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
    {
        RenderStaticMeshes(Level, ActiveViewport);
    }
    RenderGizmos(Level, ActiveViewport);
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
    {
        RenderBillboards(Level, ActiveViewport);
        RenderTexts(Level, ActiveViewport);
    }
    RenderLight(Level, ActiveViewport);
    
    ClearRenderArr();
}

void FRenderer::RenderStaticMeshes(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader(TEXT("StaticMesh"));
    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );
        // 최종 MVP 행렬
        FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        // 노말 회전시 필요 행렬
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = StaticMeshComp->EncodeUUID() / 255.0f;
        if (Level->GetSelectedActor() == StaticMeshComp->GetOwner())
        {
            //UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
        }
        else
            //UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

        if (USkySphereComponent* skysphere = Cast<USkySphereComponent>(StaticMeshComp))
        {
            //UpdateTextureConstant(skysphere->UOffset, skysphere->VOffset);
        }
        else
        {
            //UpdateTextureConstant(0, 0);
        }

        if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            UPrimitiveBatch::GetInstance().RenderAABB(
                StaticMeshComp->GetBoundingBox(),
                StaticMeshComp->GetWorldLocation(),
                Model
            );
        }
                
    
        if (!StaticMeshComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        //RenderPrimitive(renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    }
}

void FRenderer::RenderGizmos(const ULevel* Level, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!Level->GetSelectedActor())
    {
        return;
    }

    #pragma region GizmoDepth
        const Microsoft::WRL::ComPtr<ID3D11DepthStencilState> DepthStateDisable = Graphics->DepthStateDisable;
        Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable.Get(), 0);
    #pragma endregion GizmoDepth

    //  fill solid,  Wirframe 에서도 제대로 렌더링되기 위함
    Graphics->DeviceContext->RSSetState(FEngineLoop::graphicDevice.RasterizerStateSOLID.Get());
    
    for (auto GizmoComp : GizmoObjs)
    {
        
        if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ArrowZ)
            && Level->GetEditorPlayer()->GetControlMode() != CM_TRANSLATION)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::ScaleZ)
            && Level->GetEditorPlayer()->GetControlMode() != CM_SCALE)
            continue;
        else if ((GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleX ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleY ||
            GizmoComp->GetGizmoType()==UGizmoBaseComponent::CircleZ)
            && Level->GetEditorPlayer()->GetControlMode() != CM_ROTATION)
            continue;
        FMatrix Model = JungleMath::CreateModelMatrix(GizmoComp->GetWorldLocation(),
            GizmoComp->GetWorldRotation(),
            GizmoComp->GetWorldScale()
        );
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = GizmoComp->EncodeUUID() / 255.0f;

        FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();

        //if (GizmoComp == Level->GetPickingGizmo())
            //UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
        //else
            //UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        //RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

    Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer().Get());

#pragma region GizmoDepth
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState>  originalDepthState = Graphics->DepthStencilState;
    Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState.Get(), 0);
#pragma endregion GizmoDepth
}

void FRenderer::RenderBillboards(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader(TEXT("Texture"));
    //PrepareSubUVConstant();
    
    for (auto BillboardComp : BillboardObjs)
    {
        //UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();

        // 최종 MVP 행렬
        FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
        // if (BillboardComp == Level->GetPickingGizmo())
        //     UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
        // else
        //     UpdateConstant(MVP, NormalMatrix, UUIDColor, false);

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            // RenderTexturePrimitive(
            //     SubUVParticle->vertexSubUVBuffer, SubUVParticle->indexTextureBuffer,
            //     SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
            // );
        }
        else
        {
            // RenderTexturePrimitive(
            //     BillboardComp->vertexTextureBuffer, BillboardComp->indexTextureBuffer,
            //     BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
            // );
        }
    }
    PrepareShader(TEXT("StaticMesh"));
}

void FRenderer::RenderTexts(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader(TEXT("Font"));
    //PrepareSubUVConstant();
    
    for (auto TextComps : TextObjs)
    {
        if (UTextBillboardComponent* Text = Cast<UTextBillboardComponent>(TextComps))
        {
            //UpdateSubUVConstant(Text->finalIndexU, Text->finalIndexV);

            FMatrix Model = Text->CreateBillboardMatrix();

            // 최종 MVP 행렬
            FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
            FVector4 UUIDColor = TextComps->EncodeUUID() / 255.0f;
            //if (TextComps == Level->GetPickingGizmo())
                //UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
            //else
                //UpdateConstant(MVP, NormalMatrix, UUIDColor, false);
            
            // FEngineLoop::renderer.RenderTextPrimitive(
            //     Text->vertexTextBuffer, Text->numTextVertices,
            //     Text->Texture->TextureSRV, Text->Texture->SamplerState
            // );
        }
        else if (UTextRenderComponent* Text = Cast<UTextRenderComponent>(TextComps))
        {
            //UpdateSubUVConstant(Text->finalIndexU, Text->finalIndexV);

            FMatrix Model = JungleMath::CreateModelMatrix(
                Text->GetWorldLocation(),
                Text->GetWorldRotation(),
                Text->GetWorldScale()
            );

            // 최종 MVP 행렬
            FMatrix MVP = Model * ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
            FVector4 UUIDColor = TextComps->EncodeUUID() / 255.0f;
            // if (TextComps == Level->GetPickingGizmo())
            //     UpdateConstant(MVP, NormalMatrix, UUIDColor, true);
            // else
            //     UpdateConstant(MVP, NormalMatrix, UUIDColor, false);
            
            // FEngineLoop::renderer.RenderTextPrimitive(
            //     Text->vertexTextBuffer, Text->numTextVertices,
            //     Text->Texture->TextureSRV, Text->Texture->SamplerState
            // );
        }
    }
    PrepareShader(TEXT("StaticMesh"));
}

void FRenderer::RenderLight(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    for (auto Light : LightObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(Light->GetWorldLocation(), Light->GetWorldRotation(), {1, 1, 1});
        UPrimitiveBatch::GetInstance().AddCone(Light->GetWorldLocation(), Light->GetRadius(), 15, 140, Light->GetColor(), Model);
        UPrimitiveBatch::GetInstance().RenderOBB(Light->GetBoundingBox(), Light->GetWorldLocation(), Model);
    }
}
