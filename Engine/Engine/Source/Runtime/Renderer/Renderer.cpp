#include "Renderer.h"
#include <d3dcompiler.h>

#include "Level.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EngineLoop.h"
#include "UnrealEd/EditorViewportClient.h"
#include "D3D11RHI/FShaderProgram.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "D3D11RHI/GPUBuffer/TestConstantDefine.h"
#include "RenderPass/BaseRenderPass.h"
#include "RenderPass/BillboardRenderPass.h"
#include "RenderPass/FinalRenderPass.h"
#include "RenderPass/FogRenderPass.h"
#include "RenderPass/FontRenderPass.h"
#include "RenderPass/GizmoRenderPass.h"
#include "RenderPass/LightingRenderPass.h"
#include "RenderPass/LineBatchRenderPass.h"
#include "RenderPass/StaticMeshRenderPass.h"
#include "Editor/PropertyEditor/ShowFlags.h"

void FRenderer::AddOrSetVertexShader(const FString& InName, ID3D11VertexShader* InShader)
{
    if (ShaderPrograms.Contains(InName) == false)
    {
        ShaderPrograms[InName] = std::make_shared<FShaderProgram>();
    }
    ShaderPrograms[InName]->SetVertexShader(InShader);
}

void FRenderer::AddOrSetPixelShader(const FString& InName, ID3D11PixelShader* InShader)
{
    if (ShaderPrograms.Contains(InName) == false)
    {
        ShaderPrograms[InName] = std::make_shared<FShaderProgram>();
    }
    ShaderPrograms[InName]->SetPixelShader(InShader);
}

void FRenderer::AddOrSetInputLayout(const FString& InName, ID3D11InputLayout* InLayout)
{
    if (ShaderPrograms.Contains(InName) == false)
    {
        ShaderPrograms[InName] = std::make_shared<FShaderProgram>();
    }
    ShaderPrograms[InName]->SetInputLayout(InLayout);
}

void FRenderer::AddOrSetVertexBuffer(const FString& InName, ID3D11Buffer* InBuffer, const uint32 InStride, const uint32 InNumVertices, const D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
    if (VIBuffers.Contains(InName) == false)
    {
        VIBuffers[InName] = std::make_shared<FVIBuffers>();
    }
    VIBuffers[InName]->SetVertexBuffer(InBuffer, InStride, InNumVertices, InTopology);    
}

void FRenderer::AddOrSetIndexBuffer(const FString& InName, ID3D11Buffer* InBuffer, const uint32 numIndices)
{
    if (VIBuffers.Contains(InName) == false)
    {
        VIBuffers[InName] = std::make_shared<FVIBuffers>();
    }
    VIBuffers[InName]->SetIndexBuffer(InBuffer, numIndices);
}

void FRenderer::AddOrSetStructuredBuffer(const FString& InName, ID3D11Buffer* InBuffer)
{
    if (StructuredBuffers.Contains(InName) == false)
    {
        StructuredBuffers[InName] = TPair<ID3D11Buffer*, ID3D11ShaderResourceView*>();
    }
    StructuredBuffers[InName].Key = InBuffer;
}

void FRenderer::AddOrSetStructuredBufferShaderResourceView(const FString& InName, ID3D11ShaderResourceView* InShaderResourceView)
{
    if (StructuredBuffers.Contains(InName) == false)
    {
        StructuredBuffers[InName] = TPair<ID3D11Buffer*, ID3D11ShaderResourceView*>();
    }
    StructuredBuffers[InName].Value = InShaderResourceView;
}

void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateStaticMeshShader();
    CreateFontShader();
    CreateTextureShader();
    CreateLightShader();
    CreateLineShader();
    CreateFogShader();
    CreateConstantBuffer();
    UpdateLitUnlitConstant(1);
    CreateDefaultPostProcessShader();

    LoadStates();
}

void FRenderer::Release()
{
    ReleaseStates();
    ReleaseConstantBuffers();
    ReleaseShaders();
}

void FRenderer::CreateStaticMeshShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;
    ID3DBlob* VSBlob_Quad = nullptr;
    ID3DBlob* PSBlob_Lighting = nullptr;
    ID3DBlob* PSBlob_Depth = nullptr;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    Graphics->CreateVertexShader(TEXT("StaticMeshVertexShader.hlsl"), &VSBlob_StaticMesh, &VertexShader);
    Graphics->CreatePixelShader(TEXT("StaticMeshPixelShader.hlsl"), &PSBlob_StaticMesh, &PixelShader);

    D3DCompileFromFile(L"Shaders/QuadVertexShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", compileFlags, 0, &VSBlob_Quad, nullptr);
    Graphics->Device->CreateVertexShader(VSBlob_Quad->GetBufferPointer(), VSBlob_Quad->GetBufferSize(), nullptr, &QuadShader);

    D3DCompileFromFile(L"Shaders/LightingPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", compileFlags, 0, &PSBlob_Lighting, nullptr);
    Graphics->Device->CreatePixelShader(PSBlob_Lighting->GetBufferPointer(), PSBlob_Lighting->GetBufferSize(), nullptr, &LightingPixelShader);

    D3DCompileFromFile(L"Shaders/QuadDepthPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", compileFlags, 0, &PSBlob_Depth, nullptr);
    Graphics->Device->CreatePixelShader(PSBlob_Depth->GetBufferPointer(), PSBlob_Depth->GetBufferSize(), nullptr, &DepthPixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_StaticMesh);
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_StaticMesh);
    
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

    ShaderPrograms.Add(TEXT("StaticMesh"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout));
    ShaderConstantNames.Add(TEXT("StaticMesh"), ShaderStageToCB);

    staticMeshRenderPass = std::make_shared<StaticMeshRenderPass>(TEXT("StaticMesh"));
    gizmoRenderPass = std::make_shared<GizmoRenderPass>(TEXT("StaticMesh"));

    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh)
}

void FRenderer::CreateLightShader()
{
    ID3DBlob* VSBlob_Quad = nullptr;
    ID3DBlob* PSBlob_Lighting = nullptr;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    
    Graphics->CreateVertexShader(TEXT("QuadVertexShader.hlsl"), &VSBlob_Quad, &VertexShader);
    Graphics->CreatePixelShader(TEXT("LightingPixelShader.hlsl"), &PSBlob_Lighting, &PixelShader);
    
    const TArray<FConstantBufferInfo> VertexQuadConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_Quad);
    const TArray<FConstantBufferInfo> PixelLightingConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_Lighting);
    
    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : VertexQuadConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PixelLightingConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    ShaderPrograms.Add(TEXT("Lighting"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, nullptr));
    ShaderConstantNames.Add(TEXT("Lighting"), ShaderStageToCB);
    
    lightingRenderPass = std::make_shared<LightingRenderPass>(TEXT("Lighting"));
    
    SAFE_RELEASE(VSBlob_Quad)
    SAFE_RELEASE(PSBlob_Lighting)
}

void FRenderer::ReleaseShader()
{
    SAFE_RELEASE(InputLayout)
    SAFE_RELEASE(VertexShader)
    SAFE_RELEASE(PixelShader)
    SAFE_RELEASE(LightingPixelShader)
}

void FRenderer::PrepareShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    Graphics->Prepare();

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &MaterialConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &LightArrConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &SubMeshConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &TextureConstantBufer);
    }
}

void FRenderer::PrepareGizmoShader() const
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);
    Graphics->PrepareGizmo();

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &ConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &MaterialConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(2, 1, &LightArrConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(3, 1, &FlagBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(4, 1, &SubMeshConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(5, 1, &TextureConstantBufer);
    }
}

void FRenderer::PrepareLightingShader() const
{
    Graphics->DeviceContext->VSSetShader(QuadShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(LightingPixelShader, nullptr, 0);
    Graphics->PrepareLighting();

    if (LightArrConstantBuffer)
    {
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &LightArrConstantBuffer);
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &TextureConstantBufer);
    }

    Graphics->DeviceContext->IASetInputLayout(nullptr); // 입력 레이아웃 불필요
    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
}

void FRenderer::PrepareDepthShader() const
{
    Graphics->DeviceContext->VSSetShader(QuadShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(DepthPixelShader, nullptr, 0);
    Graphics->PrepareDepthScene();
    
    if (TextureConstantBufer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &TextureConstantBufer);
    }
    
    Graphics->DeviceContext->IASetInputLayout(nullptr); // 입력 레이아웃 불필요
    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
}

void FRenderer::ResetVertexShader() const
{
    Graphics->DeviceContext->VSSetShader(nullptr, nullptr, 0);
    VertexShader->Release();
}

void FRenderer::ResetPixelShader() const
{
    Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);
    PixelShader->Release();
}

void FRenderer::SetVertexShader(const FWString& filename, const FString& funcname, const FString& version)
{
    // ���� �߻��� ���ɼ��� ����
    if (Graphics == nullptr)
        assert(0);
    if (VertexShader != nullptr)
        ResetVertexShader();
    if (InputLayout != nullptr)
        InputLayout->Release();
    ID3DBlob* vertexshaderCSO;

    D3DCompileFromFile(filename.c_str(), nullptr, nullptr, *funcname, *version, 0, 0, &vertexshaderCSO, nullptr);
    Graphics->Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &VertexShader);
    vertexshaderCSO->Release();
}

void FRenderer::SetPixelShader(const FWString& filename, const FString& funcname, const FString& version)
{
    // ���� �߻��� ���ɼ��� ����
    if (Graphics == nullptr)
        assert(0);
    if (VertexShader != nullptr)
        ResetVertexShader();
    ID3DBlob* pixelshaderCSO;
    D3DCompileFromFile(filename.c_str(), nullptr, nullptr, *funcname, *version, 0, 0, &pixelshaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &PixelShader);

    pixelshaderCSO->Release();
}

void FRenderer::ChangeViewMode(EViewModeIndex evi) const
{
    ID3D11SamplerState* nullSampler[1] = { nullptr };
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        UpdateLitUnlitConstant(1);
        break;
    case EViewModeIndex::VMI_Wireframe:
    case EViewModeIndex::VMI_Unlit:
        UpdateLitUnlitConstant(0);
    case EViewModeIndex::VMI_SceneDepth:
        //UpdateSceneDepthConstant();
        break;
    }
}

void FRenderer::RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex = -1) const
{
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &renderData->VertexBuffer, &Stride, &offset);

    if (renderData->IndexBuffer)
        Graphics->DeviceContext->IASetIndexBuffer(renderData->IndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    if (renderData->MaterialSubsets.Num() == 0)
    {
        // no submesh
        Graphics->DeviceContext->DrawIndexed(renderData->Indices.Num(), 0, 0);
    }

    for (int subMeshIndex = 0; subMeshIndex < renderData->MaterialSubsets.Num(); subMeshIndex++)
    {
        int materialIndex = renderData->MaterialSubsets[subMeshIndex].MaterialIndex;

        subMeshIndex == selectedSubMeshIndex ? UpdateSubMeshConstant(true) : UpdateSubMeshConstant(false);

        overrideMaterial[materialIndex] != nullptr ? 
            UpdateMaterial(overrideMaterial[materialIndex]->GetMaterialInfo()) : UpdateMaterial(materials[materialIndex]->Material->GetMaterialInfo());

        if (renderData->IndexBuffer)
        {
            // index draw
            uint64 startIndex = renderData->MaterialSubsets[subMeshIndex].IndexStart;
            uint64 indexCount = renderData->MaterialSubsets[subMeshIndex].IndexCount;
            Graphics->DeviceContext->DrawIndexed(indexCount, startIndex, 0);
        }
    }
}

void FRenderer::RenderTexturedModelPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV,
    ID3D11SamplerState* InSamplerState
) const
{
    if (!InTextureSRV || !InSamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &Stride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    //Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &InTextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &InSamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD = {vertices};

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateVertexBuffer(const TArray<FVertexSimple>& vertices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA vertexbufferSRD;
    vertexbufferSRD.pSysMem = vertices.GetData();

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;

    Graphics->CreateVertexShader(TEXT("PostProcessVertexShader.hlsl"), &VertexShaderCSO, &VertexShader);
    Graphics->CreatePixelShader(TEXT("PostProcessPixelShader.hlsl"), &PixelShaderCSO, &PixelShader);
    
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> PostProcessVertexConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO);
    const TArray<FConstantBufferInfo> PostProcessPixelConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO);

    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : PostProcessVertexConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PostProcessPixelConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    ShaderPrograms.Add(TEXT("DefaultPostProcess"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout));
    ShaderConstantNames.Add(TEXT("DefaultPostProcess"), ShaderStageToCB);
    
    finalRenderPass = std::make_shared<FinalRenderPass>(TEXT("DefaultPostProcess"));

    SAFE_RELEASE(VertexShaderCSO);
    SAFE_RELEASE(PixelShaderCSO);
}

void FRenderer::CreateTextureShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    Graphics->CreateVertexShader(TEXT("TextureVertexShader.hlsl"), &VertexShaderCSO, &VertexShader);
    Graphics->CreatePixelShader(TEXT("TexturePixelShader.hlsl"), &PixelShaderCSO, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexTextureMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO);
    const TArray<FConstantBufferInfo> PixelTextureMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO);

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

    ShaderPrograms.Add(TEXT("Texture"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout));
    ShaderConstantNames.Add(TEXT("Texture"), ShaderStageToCB);

    billboardRenderPass = std::make_shared<BillboardRenderPass>(TEXT("Texture"));
    
    SAFE_RELEASE(VertexShaderCSO);
    SAFE_RELEASE(PixelShaderCSO);
}

void FRenderer::UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const
{
    if (MaterialConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(MaterialConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FMaterialConstants* constants = static_cast<FMaterialConstants*>(ConstantBufferMSR.pData);
            constants->DiffuseColor = MaterialInfo.Diffuse;
            constants->TransparencyScalar = MaterialInfo.TransparencyScalar;
            constants->AmbientColor = MaterialInfo.Ambient;
            constants->DensityScalar = MaterialInfo.DensityScalar;
            constants->SpecularColor = MaterialInfo.Specular;
            constants->SpecularScalar = MaterialInfo.SpecularScalar;
            constants->EmmisiveColor = MaterialInfo.Emissive;
        }
        Graphics->DeviceContext->Unmap(MaterialConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }

    if (MaterialInfo.bHasTexture == true)
    {
        if (MaterialInfo.DiffuseTextureName != "")
        {
            std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.DiffuseTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(0, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        if (MaterialInfo.SpecularTextureName != "")
        {
            std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.SpecularTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(1, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
        if (MaterialInfo.BumpTextureName != "")
        {
            std::shared_ptr<FTexture> texture = FEngineLoop::resourceMgr.GetTexture(MaterialInfo.BumpTexturePath);
            Graphics->DeviceContext->PSSetShaderResources(2, 1, &texture->TextureSRV);
            Graphics->DeviceContext->PSSetSamplers(0, 1, &texture->SamplerState);
        }
    }
    else
    {
        ID3D11ShaderResourceView* nullSRV[1] = {nullptr};
        ID3D11SamplerState* nullSampler[1] = {nullptr};
        
        Graphics->DeviceContext->PSSetShaderResources(0, 1, nullSRV);
        Graphics->DeviceContext->PSSetSamplers(0, 1, nullSampler);
    }
}

void FRenderer::UpdateLightBuffer(TArray<ULightComponent*> lightComponents) const
{
    if (!LightArrConstantBuffer) return;
    
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(LightArrConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    {
        FLightingArr* constants = static_cast<FLightingArr*>(mappedResource.pData);
        constants->EyePosition = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->ViewTransformPerspective.ViewLocation;
        constants->LightCount = lightComponents.Num();
        for (int index = 0; index< lightComponents.Num();index++)
        {
            constants->Lights[index].Intensity = lightComponents[index]->GetIntensity();
            constants->Lights[index].Position = lightComponents[index]->GetWorldLocation();
            constants->Lights[index].AmbientFactor = 0.0f;
            constants->Lights[index].LightColor = lightComponents[index]->GetLightColor();
            constants->Lights[index].LightDirection = FVector(-1,-1,-1);
            constants->Lights[index].AttenuationRadius = lightComponents[index]->GetAttenuationRadius();
        }
    }
    Graphics->DeviceContext->Unmap(LightArrConstantBuffer, 0);
}

void FRenderer::UpdateConstant(const FMatrix& Model, const FMatrix& ViewProjection, const FMatrix& NormalMatrix, bool IsSelected) const
{
    if (ConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE ConstantBufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &ConstantBufferMSR); // update constant buffer every frame
        {
            FConstants* constants = static_cast<FConstants*>(ConstantBufferMSR.pData);
            constants->Model = Model;
            constants->ViewProjection = ViewProjection;
            constants->ModelMatrixInverseTranspose = NormalMatrix;
            constants->IsSelected = IsSelected;
        }
        Graphics->DeviceContext->Unmap(ConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::CreateFontShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    Graphics->CreateVertexShader(TEXT("FontVertexShader.hlsl"), &VertexShaderCSO, &VertexShader);

    Graphics->CreatePixelShader(TEXT("FontPixelShader.hlsl"), &PixelShaderCSO, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexFontMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO);
    const TArray<FConstantBufferInfo> PixelFontMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO);

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

    ShaderPrograms.Add(TEXT("Font"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout));
    ShaderConstantNames.Add(TEXT("Font"), ShaderStageToCB);

    fontRenderPass = std::make_shared<FontRenderPass>(TEXT("Font"));
    
    SAFE_RELEASE(VertexShaderCSO);
    SAFE_RELEASE(PixelShaderCSO);
}

void FRenderer::CreateLineShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    Graphics->CreateVertexShader(TEXT("ShaderLineVertexShader.hlsl"),&VertexShaderCSO, &VertexShader);
    Graphics->CreatePixelShader(TEXT("ShaderLinePixelShader.hlsl"), &PixelShaderCSO, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        // 정점 ID: 32비트 부호 없는 정수, 입력 슬로트 0, Per-Vertex 데이터
        { "SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    
        // 인스턴스 ID: 32비트 부호 없는 정수, 입력 슬로트 1, Per-Instance 데이터
        { "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    
    Graphics->Device->CreateInputLayout(
    layoutDesc, ARRAYSIZE(layoutDesc), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexLineConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO);
    const TArray<FConstantBufferInfo> PixelLineConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO);

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
    
    ShaderPrograms.Add(TEXT("Line"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout));
    ShaderConstantNames.Add(TEXT("Line"), ShaderStageToCB);

    lineBatchRenderPass = std::make_shared<LineBatchRenderPass>(TEXT("Line"));
    
    SAFE_RELEASE(VertexShaderCSO);
    SAFE_RELEASE(PixelShaderCSO);
}

void FRenderer::CreateFogShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    Graphics->CreateVertexShader(TEXT("PostProcessVertexShader.hlsl"),&VertexShaderCSO, &VertexShader);
    Graphics->CreatePixelShader(TEXT("FogPixelShader.hlsl"), &PixelShaderCSO, &PixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &InputLayout
    );
        
    const TArray<FConstantBufferInfo> VertexFogConstant = FGraphicsDevice::ExtractConstantBufferNames(VertexShaderCSO);
    const TArray<FConstantBufferInfo> PixelFogConstant = FGraphicsDevice::ExtractConstantBufferNames(PixelShaderCSO);

    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const auto item : VertexFogConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }

    for (const auto item :PixelFogConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        ConstantBuffers[item.Name] = CreateConstantBuffer(item.ByteWidth);
    }
    
    ShaderPrograms.Add(TEXT("Fog"), std::make_shared<FShaderProgram>(VertexShader, PixelShader, InputLayout));
    ShaderConstantNames.Add(TEXT("Fog"), ShaderStageToCB);

    fogRenderPass = std::make_shared<FogRenderPass>(TEXT("Fog"));
    
    SAFE_RELEASE(VertexShaderCSO);
    SAFE_RELEASE(PixelShaderCSO);
}

void FRenderer::LoadStates()
{
#pragma region sampler state
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics->Device->CreateSamplerState(&samplerDesc, &SamplerStates[static_cast<uint32>(ESamplerType::Anisotropic)]);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&samplerDesc, &SamplerStates[static_cast<uint32>(ESamplerType::Point)]);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&samplerDesc, &SamplerStates[static_cast<uint32>(ESamplerType::Linear)]);

	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    Graphics->Device->CreateSamplerState(&samplerDesc, &SamplerStates[static_cast<uint32>(ESamplerType::PostProcess)]);

	Graphics->BindSamplers(static_cast<uint32>(ESamplerType::Point), 1, &SamplerStates[static_cast<uint32>(ESamplerType::Point)]);
	Graphics->BindSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &SamplerStates[static_cast<uint32>(ESamplerType::Linear)]);
	Graphics->BindSamplers(static_cast<uint32>(ESamplerType::Anisotropic), 1, &SamplerStates[static_cast<uint32>(ESamplerType::Anisotropic)]);
	Graphics->BindSamplers(static_cast<uint32>(ESamplerType::PostProcess), 1, &SamplerStates[static_cast<uint32>(ESamplerType::PostProcess)]);
#pragma endregion
#pragma region rasterize state
    D3D11_RASTERIZER_DESC rsDesc = {};
    rsDesc.AntialiasedLineEnable = false;
    rsDesc.CullMode = D3D11_CULL_BACK;
    rsDesc.DepthBias = 0;
    rsDesc.DepthBiasClamp = 0.0f;
    rsDesc.DepthClipEnable = true;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.FrontCounterClockwise = false;
    rsDesc.MultisampleEnable = false;
    rsDesc.ScissorEnable = false;
    rsDesc.SlopeScaledDepthBias = 0.0f;
    Graphics->CreateRasterizerState(
        &rsDesc, &RasterizerStates[static_cast<uint32>(ERasterizerState::SolidBack)]);

    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_FRONT;
    Graphics->CreateRasterizerState(
        &rsDesc, &RasterizerStates[static_cast<uint32>(ERasterizerState::SolidFront)]);

    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    Graphics->CreateRasterizerState(
        &rsDesc, &RasterizerStates[static_cast<uint32>(ERasterizerState::SolidNone)]);

    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    rsDesc.CullMode = D3D11_CULL_NONE;
    Graphics->CreateRasterizerState(
        &rsDesc, &RasterizerStates[static_cast<uint32>(ERasterizerState::WireFrame)]);
#pragma endregion
#pragma region blend state
    D3D11_BLEND_DESC bsDesc = {};
    bsDesc.AlphaToCoverageEnable = false;
    bsDesc.IndependentBlendEnable = false;
    bsDesc.RenderTarget[0].BlendEnable = true;
    bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    bsDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    bsDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    bsDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    Graphics->CreateBlendState(&bsDesc, &BlendStates[static_cast<uint32>(EBlendState::AlphaBlend)]);

    bsDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    bsDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    Graphics->CreateBlendState(&bsDesc, &BlendStates[static_cast<uint32>(EBlendState::OneOne)]);
#pragma endregion
#pragma region depthstencil state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = true;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    Graphics->CreateDepthStencilState(
        &dsDesc, &DepthStencilStates[static_cast<uint32>(EDepthStencilState::LessEqual)]);

    dsDesc.DepthEnable = false;
    dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    dsDesc.StencilEnable = false;
    Graphics->CreateDepthStencilState(
        &dsDesc, &DepthStencilStates[static_cast<uint32>(EDepthStencilState::DepthNone)]);
#pragma endregion
}

void FRenderer::ReleaseShaders()
{
    for (const auto item : ShaderPrograms)
    {
        item.Value->Release();
    }
}

void FRenderer::ReleaseConstantBuffers()
{
    for (auto item : ConstantBuffers)
    {
        if (item.Value != nullptr)
        {
            item.Value->Release();
            item.Value = nullptr;
        }
    }
}

void FRenderer::ReleaseStates()
{
    for (auto item : SamplerStates)
    {
        if (item != nullptr)
        {
            item->Release();
            item = nullptr;
        }
    }

    for (auto item : RasterizerStates)
    {
        if (item != nullptr)
        {
            item->Release();
            item = nullptr;
        }
    }

    for (auto item : BlendStates)
    {
        if (item != nullptr)
        {
            item->Release();
            item = nullptr;
        }
    }

    for (auto item : DepthStencilStates)
    {
        if (item != nullptr)
        {
            item->Release();
            item = nullptr;
        }
    }
}

void FRenderer::ReBindSamplers() const
{
    Graphics->BindSamplers(static_cast<uint32>(ESamplerType::Point), 1,
                              &SamplerStates[static_cast<uint32>(ESamplerType::Point)]);
    Graphics->BindSamplers(static_cast<uint32>(ESamplerType::Linear), 1,
                              &SamplerStates[static_cast<uint32>(ESamplerType::Linear)]);
    Graphics->BindSamplers(static_cast<uint32>(ESamplerType::Anisotropic), 1,
                              &SamplerStates[static_cast<uint32>(ESamplerType::Anisotropic)]);
    Graphics->BindSamplers(static_cast<uint32>(ESamplerType::PostProcess), 1,
                              &SamplerStates[static_cast<uint32>(ESamplerType::PostProcess)]);
}

void FRenderer::PrepareShader(const FString& InShaderName) const
{
    ShaderPrograms[InShaderName]->Bind(Graphics->DeviceContext);

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
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        bIsLit = true;
        SetCurrentRasterizerState(ERasterizerState::SolidBack);
        break;
    case EViewModeIndex::VMI_Wireframe:
        SetCurrentRasterizerState(ERasterizerState::WireFrame);
        break;
    case EViewModeIndex::VMI_Unlit:
        bIsLit = false;
        SetCurrentRasterizerState(ERasterizerState::SolidBack);
        break;
    }
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(const uint32* indices, const uint32 indicesSize) const
{
    TArray<uint32> indicesToCopy;
    indicesToCopy.AppendArray(indices, indicesSize);

    return CreateIndexBuffer(indicesToCopy);
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(const TArray<uint32>& indices) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.ByteWidth = sizeof(uint32) * indices.Num();

    D3D11_SUBRESOURCE_DATA indexbufferSRD;
    indexbufferSRD.pSysMem = indices.GetData();

   ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

ID3D11Buffer* FRenderer::CreateConstantBuffer(const uint32 InSize, const void* InData) const
{
    D3D11_BUFFER_DESC constantBufferDesc = {};   
    constantBufferDesc.ByteWidth = InSize;
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D11_SUBRESOURCE_DATA sub = {};
    sub.pSysMem = InData;
    
    ID3D11Buffer* constantBuffer;

    HRESULT hr;
    if (InData == nullptr)
        hr = Graphics->Device->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);
    else
        hr = Graphics->Device->CreateBuffer(&constantBufferDesc, &sub, &constantBuffer);

    if (FAILED(hr))
        assert(NULL/*"Create constant buffer failed!"*/);

    return constantBuffer;
}

ID3D11ShaderResourceView* FRenderer::CreateBufferSRV(ID3D11Buffer* pBuffer, const uint32 numElements) const
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // 구조화된 버퍼의 경우 형식은 UNKNOWN으로 지정
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numElements;

    ID3D11ShaderResourceView* pSRV = nullptr;
    const HRESULT hr = Graphics->Device->CreateShaderResourceView(pBuffer, &srvDesc, &pSRV);
    if (FAILED(hr))
    {
        // 오류 처리 (필요에 따라 로그 출력 등)
        assert(false && "CreateStructuredBufferShaderResourceView failed");
        return nullptr;
    }
    return pSRV;
}


    Graphics->Device->CreateShaderResourceView(pConeBuffer, &srvDesc, &pConeSRV);
    return pConeSRV;
}

void FRenderer::UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FBoundingBox*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes, int numBoundingBoxes) const
{
    if (!pBoundingBoxBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pBoundingBoxBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FOBB*>(mappedResource.pData);
    for (int i = 0; i < BoundingBoxes.Num(); ++i)
    {
        pData[i] = BoundingBoxes[i];
    }
    Graphics->DeviceContext->Unmap(pBoundingBoxBuffer, 0);
}

void FRenderer::UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const
{
    if (!pConeBuffer) return;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(pConeBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = reinterpret_cast<FCone*>(mappedResource.pData);
    for (int i = 0; i < Cones.Num(); ++i)
    {
        pData[i] = Cones[i];
    }
    Graphics->DeviceContext->Unmap(pConeBuffer, 0);
}

void FRenderer::UpdateGridConstantBuffer(const FGridParameters& gridParams) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(GridConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr))
    {
        memcpy(mappedResource.pData, &gridParams, sizeof(FGridParameters));
        Graphics->DeviceContext->Unmap(GridConstantBuffer, 0);
    }
    else
    {
        UE_LOG(LogLevel::Warning, "gridParams ���� ����");
    }
}

void FRenderer::UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const
{
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = Graphics->DeviceContext->Map(LinePrimitiveBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    auto pData = static_cast<FPrimitiveCounts*>(mappedResource.pData);
    pData->BoundingBoxCount = numBoundingBoxes;
    pData->ConeCount = numCones;
    Graphics->DeviceContext->Unmap(LinePrimitiveBuffer, 0);
}

void FRenderer::RenderBatch(
    const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount
) const
{ 
    UINT stride = sizeof(FSimpleVertex);
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &stride, &offset);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT vertexCountPerInstance = 2;
    UINT instanceCount = gridParam.numGridLines + 3 + (boundingBoxCount * 12) + (coneCount * (2 * coneSegmentCount)) + (12 * obbCount);
    Graphics->DeviceContext->DrawInstanced(vertexCountPerInstance, instanceCount, 0, 0);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void FRenderer::SetRenderObj(ULevel* Level)
{
    TArray<USceneComponent*> Ss;
    for (const auto& A : Level->GetActors())
    {
        Ss.Add(A->GetRootComponent());
        TArray<UActorComponent*> components;
        components = A->GetComponents();
        for (const auto& comp : components)
        {
            if (ULightComponent* pLightComp = Cast<ULightComponent>(comp))
                LightObjs.Add(pLightComp);
            if (UHeightFogComponent* pHeightFogComp = Cast<UHeightFogComponent>(comp))
                HeightFogObjs.Add(pHeightFogComp);
            if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(comp))
                GizmoObjs.Add(pGizmoComp);
            if (UTextRenderComponent* TextRenderComp = Cast<UTextRenderComponent>(comp))
                TextObjs.Add(TextRenderComp);
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
    HeightFogObjs.Empty();
}

void FRenderer::Render(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    ChangeViewMode(ActiveViewport->GetViewMode());
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
    {
        RenderStaticMeshes(Level, ActiveViewport);
    } 
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
    {
        RenderBillboards(Level, ActiveViewport);
        RenderTexts(Level, ActiveViewport);
    }
    // --- SceneDepth 특별 처리 ---
    if (ActiveViewport->GetViewMode() == EViewModeIndex::VMI_SceneDepth)
    {
        RenderDepthScene(Level, ActiveViewport);
    }
    else if (ActiveViewport->ViewMode == VMI_Lit)
    {
        RenderLighting(Level, ActiveViewport);
    }
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_HeightFog))
    {
        RenderFog(Level, ActiveViewport);
    }
    
    RenderGizmos(Level, ActiveViewport);
    Graphics->PrepareGridRender();
    UPrimitiveBatch::GetInstance().RenderBatch(ActiveViewport->GetViewMatrix(), ActiveViewport->GetProjectionMatrix());

    RenderFinal(Level, ActiveViewport);
    ClearRenderArr();

}

void FRenderer::RenderStaticMeshes(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareShader();
    for (UStaticMeshComponent* StaticMeshComp : StaticMeshObjs)
    {
        FMatrix Model = JungleMath::CreateModelMatrix(
            StaticMeshComp->GetWorldLocation(),
            StaticMeshComp->GetWorldRotation(),
            StaticMeshComp->GetWorldScale()
        );
        // 최종 MVP 행렬
        FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        // 노말 회전시 필요 행렬
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));

        UpdateConstant(Model, VP, NormalMatrix, Level->GetSelectedActor() == StaticMeshComp->GetOwner());

        if (USkySphereComponent* skysphere = Cast<USkySphereComponent>(StaticMeshComp))
        {
            UpdateTextureConstant(skysphere->UOffset, skysphere->VOffset,1,1);
        }
        else
        {
            UpdateTextureConstant(0, 0,1,1);
        }

        if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_AABB))
        {
            if (Level->GetSelectedActor() == StaticMeshComp->GetOwner())
            {
                UPrimitiveBatch::GetInstance().RenderAABB(
                    StaticMeshComp->GetBoundingBox(),
                    StaticMeshComp->GetWorldLocation(),
                    Model
                );
            }
        }
    
        if (!StaticMeshComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = StaticMeshComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(renderData, StaticMeshComp->GetStaticMesh()->GetMaterials(), StaticMeshComp->GetOverrideMaterials(), StaticMeshComp->GetselectedSubMeshIndex());
    }
}

void FRenderer::RenderGizmos(const ULevel* Level, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    if (!Level->GetSelectedActor())
    {
        return;
    }
    PrepareGizmoShader();
    
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

        FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        
        UpdateConstant(Model, VP, NormalMatrix, GizmoComp == Level->GetPickingGizmo());

        if (!GizmoComp->GetStaticMesh()) continue;

        OBJ::FStaticMeshRenderData* renderData = GizmoComp->GetStaticMesh()->GetRenderData();
        if (renderData == nullptr) continue;

        RenderPrimitive(renderData, GizmoComp->GetStaticMesh()->GetMaterials(), GizmoComp->GetOverrideMaterials());
    }

//     Graphics->DeviceContext->RSSetState(Graphics->GetCurrentRasterizer());
//
// #pragma region GizmoDepth
//     ID3D11DepthStencilState* originalDepthState = Graphics->DepthStencilState;
//     Graphics->DeviceContext->OMSetDepthStencilState(originalDepthState, 0);
// #pragma endregion GizmoDepth
}

void FRenderer::RenderBillboards(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareTextureShader();
    PrepareSubUVConstant();
    
    for (auto BillboardComp : BillboardObjs)
    {
        UpdateSubUVConstant(BillboardComp->finalIndexU, BillboardComp->finalIndexV);

        FMatrix Model = BillboardComp->CreateBillboardMatrix();

        // 최종 MVP 행렬
        FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
        FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
        
        UpdateConstant(Model, VP, NormalMatrix, BillboardComp == Level->GetPickingGizmo());

        if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
        {
            RenderTexturePrimitive(
                SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
                SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
            );
        }
        else
        {
            RenderTexturePrimitive(
                BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
                BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
            );
        }
    }
    PrepareShader();
}

void FRenderer::RenderTexts(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    PrepareFontShader();
    PrepareSubUVConstant();
    
    for (auto TextComps : TextObjs)
    {
        if (UTextBillboardComponent* Text = Cast<UTextBillboardComponent>(TextComps))
        {
            UpdateSubUVConstant(Text->finalIndexU, Text->finalIndexV);

            FMatrix Model = Text->CreateBillboardMatrix();

            // 최종 MVP 행렬
            FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
            
            UpdateConstant(Model, VP, NormalMatrix, TextComps == Level->GetPickingGizmo());
            
            FEngineLoop::renderer.RenderTextPrimitive(
                Text->vertexTextBuffer, Text->numTextVertices,
                Text->Texture->TextureSRV, Text->Texture->SamplerState
            );
        }
        else if (UTextRenderComponent* Text = Cast<UTextRenderComponent>(TextComps))
        {
            UpdateSubUVConstant(Text->finalIndexU, Text->finalIndexV);

            FMatrix Model = JungleMath::CreateModelMatrix(
                Text->GetWorldLocation(),
                Text->GetWorldRotation(),
                Text->GetWorldScale()
            );

            // 최종 MVP 행렬
            FMatrix VP = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
            FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));

            UpdateConstant(Model, VP, NormalMatrix, TextComps == Level->GetPickingGizmo());
            
            FEngineLoop::renderer.RenderTextPrimitive(
                Text->vertexTextBuffer, Text->numTextVertices,
                Text->Texture->TextureSRV, Text->Texture->SamplerState
            );
        }
    }
    PrepareShader();
}

void FRenderer::RenderLighting(ULevel* Level, std::shared_ptr<FEditorViewportClient>& ActiveViewport) const
{
    PrepareLightingShader();

    float uoffset = ActiveViewport->Viewport->GetViewport().TopLeftX / Graphics->screenWidth;
    float voffset = ActiveViewport->Viewport->GetViewport().TopLeftY / Graphics->screenHeight;
    float uscale = ActiveViewport->Viewport->GetViewport().Width / Graphics->screenWidth;
    float vscale = ActiveViewport->Viewport->GetViewport().Height / Graphics->screenHeight;
    UpdateTextureConstant(uoffset, voffset, uscale, vscale);
    UpdateLightBuffer(LightObjs);
    
    // 화면 크기 사각형 렌더링
    Graphics->DeviceContext->Draw(6, 0); // 4개의 정점으로 화면 전체 사각형 그리기
    
    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[4] = { nullptr, nullptr, nullptr, nullptr };
    Graphics->DeviceContext->PSSetShaderResources(0, 4, nullSRV);

    // Sampler 해제
    ID3D11SamplerState* nullSamplers[1] = { nullptr };
    Graphics->DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}

void FRenderer::RenderDepthScene(ULevel* Level, std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    PrepareDepthShader();

    float uoffset = ActiveViewport->Viewport->GetViewport().TopLeftX / Graphics->screenWidth;
    float voffset = ActiveViewport->Viewport->GetViewport().TopLeftY / Graphics->screenHeight;
    float uscale = ActiveViewport->Viewport->GetViewport().Width / Graphics->screenWidth;
    float vscale = ActiveViewport->Viewport->GetViewport().Height / Graphics->screenHeight;
    UpdateTextureConstant(uoffset, voffset, uscale, vscale);
    
    // 화면 크기 사각형 렌더링
    Graphics->DeviceContext->Draw(6, 0); // 4개의 정점으로 화면 전체 사각형 그리기
    
    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV = nullptr;
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &nullSRV);

    // Sampler 해제
    ID3D11SamplerState* nullSamplers = nullptr;
    Graphics->DeviceContext->PSSetSamplers(0, 1, &nullSamplers);
}

void FRenderer::CreateDefaultPostProcessShader()
{
    ID3DBlob* PixelShaderCSO;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
    D3DCompileFromFile(L"Shaders/PostProcessPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "MainPS", "ps_5_0", flags, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(
        PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &PostProcessPixelShader
    );
    PixelShaderCSO->Release();
}

void FRenderer::AddRenderObjectsToRenderPass(const ULevel* InLevel) const
{
    // for (const auto renderPass : RenderPasses)
    // {
    //     renderPass->AddRenderObjectsToRenderPass(InLevel);
    // }
    
    staticMeshRenderPass->AddRenderObjectsToRenderPass(InLevel);
    gizmoRenderPass->AddRenderObjectsToRenderPass(InLevel);
    lineBatchRenderPass->AddRenderObjectsToRenderPass(InLevel);
    fontRenderPass->AddRenderObjectsToRenderPass(InLevel);
    billboardRenderPass->AddRenderObjectsToRenderPass(InLevel);
    lightingRenderPass->AddRenderObjectsToRenderPass(InLevel);
    fogRenderPass->AddRenderObjectsToRenderPass(InLevel);
    finalRenderPass->AddRenderObjectsToRenderPass(InLevel);
}

void FRenderer::AddRenderObjectsToRenderPass(const ULevel* InLevel) const
{
    if (PostProcessPixelShader)
    {
        PostProcessPixelShader->Release();
        PostProcessPixelShader = nullptr;
    }
}

void FRenderer::PrepareDefaultPostProcessShader() const
{
    Graphics->DeviceContext->VSSetShader(QuadShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PostProcessPixelShader, nullptr, 0);
}

void FRenderer::Render(ULevel* InLevel, std::shared_ptr<FEditorViewportClient> InActiveViewport)
{
    ID3DBlob* PixelShaderCSO;
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;
    D3DCompileFromFile(L"Shaders/FogPixelShader.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "MainPS", "ps_5_0", flags, 0, &PixelShaderCSO, nullptr);
    Graphics->Device->CreatePixelShader(
        PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &FogPixelShader
    );
    
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseFogShader()
{
    if (FogPixelShader)
    {
        FogPixelShader->Release();
        FogPixelShader = nullptr;
    }
    if (FogConstantBuffer)
    {
        lightingRenderPass->Prepare(InActiveViewport);
        lightingRenderPass->Execute(InActiveViewport);
    }
}

void FRenderer::PrepareFogShader() const
{
    Graphics->DeviceContext->VSSetShader(QuadShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(FogPixelShader, nullptr, 0);
    Graphics->PreparePostProcessRender();
    if (FogConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &TextureConstantBufer);
        Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &FogConstantBuffer);
    }
}

void FRenderer::UpdateFogConstant(UHeightFogComponent* FogComponent, const FMatrix& InvProjectionMatrix, const FMatrix& InvViewMatrix, const FVector CameraPosition)
{
    Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &FogConstantBuffer);
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    Graphics->DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

    FFogConstants* data = reinterpret_cast<FFogConstants*>(mappedResource.pData);
    data->FogDensity = FogComponent->FogDensity;
    data->FogHeightFalloff = FogComponent->FogHeightFalloff;
    data->StartDistance = FogComponent->StartDistance;
    data->FogCutoffDistance = FogComponent->FogCutoffDistance;
    data->FogMaxOpacity = FogComponent->FogMaxOpacity;
    data->FogInscatteringColor = FogComponent->FogInscatteringColor;

    data->CameraPosition = CameraPosition;
    data->InvProjectionMatrix = InvProjectionMatrix;
    data->InvViewMatrix = InvViewMatrix;

    Graphics->DeviceContext->Unmap(FogConstantBuffer, 0);
}

void FRenderer::RenderFog(ULevel* level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{   
    PrepareFogShader();
    
    UpdateFogConstant(
        Cast<UHeightFogComponent>(level->GetFog()->GetRootComponent()),
        FMatrix::Inverse(ActiveViewport->GetProjectionMatrix()),
        FMatrix::Inverse(ActiveViewport->GetViewMatrix()),
        ActiveViewport->ViewTransformPerspective.GetLocation()
    );
    
    float uoffset = ActiveViewport->Viewport->GetViewport().TopLeftX / Graphics->screenWidth;
    float voffset = ActiveViewport->Viewport->GetViewport().TopLeftY / Graphics->screenHeight;
    float uscale = ActiveViewport->Viewport->GetViewport().Width / Graphics->screenWidth;
    float vscale = ActiveViewport->Viewport->GetViewport().Height / Graphics->screenHeight;
    UpdateTextureConstant(uoffset, voffset, uscale, vscale);

    // SceneColor + Depth SRV 바인딩
    ID3D11ShaderResourceView* SRVs[] = { Graphics->GetReadSRV(), Graphics->DepthStencilSRV };
    Graphics->DeviceContext->PSSetShaderResources(0, 2, SRVs);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Graphics->SamplerState);
    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);

    // 풀스크린 쿼드 그리기
    Graphics->DeviceContext->Draw(6, 0);

    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    Graphics->DeviceContext->PSSetShaderResources(0, 2, nullSRV);

    // Sampler 해제
    ID3D11SamplerState* nullSamplers[1] = { nullptr };
    Graphics->DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}

void FRenderer::RenderFinal(ULevel* level, std::shared_ptr<FEditorViewportClient> ActiveViewport)
{
    Graphics->PrepareFinalRender();
    PrepareDefaultPostProcessShader();

    // SceneColor + Depth SRV 바인딩
    ID3D11ShaderResourceView* SRVs[2] = { Graphics->GetReadSRV(), Graphics->DepthStencilSRV};
    Graphics->DeviceContext->PSSetShaderResources(0, 2, SRVs);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Graphics->SamplerState);
    
    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R16_UINT, 0);

    if (TextureConstantBufer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0,1, &TextureConstantBufer);
    }

    float uoffset = ActiveViewport->Viewport->GetViewport().TopLeftX / Graphics->screenWidth;
    float voffset = ActiveViewport->Viewport->GetViewport().TopLeftY / Graphics->screenHeight;
    float uscale = ActiveViewport->Viewport->GetViewport().Width / Graphics->screenWidth;
    float vscale = ActiveViewport->Viewport->GetViewport().Height / Graphics->screenHeight;
    UpdateTextureConstant(uoffset, voffset, uscale, vscale);
    
    // 풀스크린 쿼드 그리기
    Graphics->DeviceContext->Draw(6, 0);

    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    Graphics->DeviceContext->PSSetShaderResources(0, 2, nullSRV);

    // Sampler 해제
    ID3D11SamplerState* nullSamplers[1] = { nullptr };
    Graphics->DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}