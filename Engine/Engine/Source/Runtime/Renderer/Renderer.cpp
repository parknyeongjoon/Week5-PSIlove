#include "Renderer.h"
#include <d3dcompiler.h>

#include "Level.h"
#include "UnrealClient.h"
#include "Actors/Player.h"
#include "Actors/Fog.h"
#include "BaseGizmos/GizmoBaseComponent.h"
#include "BaseGizmos/TransformGizmo.h"
#include "Components/LightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BillboardComponent.h"
#include "Components/ParticleSubUVComp.h"
#include "Components/TextBillboardComponent.h"
#include "Components/Material/Material.h"
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
#include "Components/HeightFogComponent.h"
#include "ImGUI/imgui_internal.h"
#include "LevelEditor/SLevelEditor.h"


void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    CreateShader();
    CreateTextureShader();
    CreateFontShader();
    CreateLineShader();
    CreateDefaultPostProcessShader();
    CreateFogShader();
    CreateConstantBuffer();
    UpdateLitUnlitConstant(1);
}

void FRenderer::Release()
{
    ReleaseShader();
    ReleaseTextureShader();
    ReleaseFontShader();
    ReleaseLineShader();
    ReleaseConstantBuffer();
    ReleaseDefaultPostProcessShader();
    ReleaseFogShader();
}

void FRenderer::CreateShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;
    ID3DBlob* VSBlob_Quad = nullptr;
    ID3DBlob* PSBlob_Lighting = nullptr;
    ID3DBlob* PSBlob_Depth = nullptr;
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    D3DCompileFromFile(L"Shaders/StaticMeshVertexShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", compileFlags, 0, &VSBlob_StaticMesh, nullptr);
    Graphics->Device->CreateVertexShader(VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), nullptr, &VertexShader);

    D3DCompileFromFile(L"Shaders/StaticMeshPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", compileFlags, 0, &PSBlob_StaticMesh, nullptr);
    Graphics->Device->CreatePixelShader(PSBlob_StaticMesh->GetBufferPointer(), PSBlob_StaticMesh->GetBufferSize(), nullptr, &PixelShader);

    D3DCompileFromFile(L"Shaders/QuadVertexShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", compileFlags, 0, &VSBlob_Quad, nullptr);
    Graphics->Device->CreateVertexShader(VSBlob_Quad->GetBufferPointer(), VSBlob_Quad->GetBufferSize(), nullptr, &QuadShader);

    D3DCompileFromFile(L"Shaders/LightingPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", compileFlags, 0, &PSBlob_Lighting, nullptr);
    Graphics->Device->CreatePixelShader(PSBlob_Lighting->GetBufferPointer(), PSBlob_Lighting->GetBufferSize(), nullptr, &LightingPixelShader);

    D3DCompileFromFile(L"Shaders/QuadDepthPixelShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", compileFlags, 0, &PSBlob_Depth, nullptr);
    Graphics->Device->CreatePixelShader(PSBlob_Depth->GetBufferPointer(), PSBlob_Depth->GetBufferSize(), nullptr, &DepthPixelShader);

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );

    Stride = sizeof(FVertexSimple);
    
    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh)
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

ID3D11Buffer* FRenderer::CreateIndexBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              // buffer�� ����, �뵵 ���� ����
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       // immutable: gpu�� �б� �������� ������ �� �ִ�.
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // index buffer�� ����ϰڴ�.
    indexbufferdesc.ByteWidth = byteWidth;               // buffer ũ�� ����

    D3D11_SUBRESOURCE_DATA indexbufferSRD = {indices};

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, &indexbufferSRD, &indexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "IndexBuffer Creation faild");
    }
    return indexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexBuffer(const TArray<uint32>& indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};              // buffer�� ����, �뵵 ���� ����
    indexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE;       // immutable: gpu�� �б� �������� ������ �� �ִ�.
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER; // index buffer�� ����ϰڴ�.
    indexbufferdesc.ByteWidth = byteWidth;               // buffer ũ�� ����

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

void FRenderer::ReleaseBuffer(ID3D11Buffer*& Buffer) const
{
    if (Buffer)
    {
        Buffer->Release();
        Buffer = nullptr;
    }
}

void FRenderer::CreateConstantBuffer()
{
    D3D11_BUFFER_DESC constantbufferdesc = {};
    constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0;
    constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FSubUVConstant) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &SubUVConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FGridParameters) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &GridConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FPrimitiveCounts) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &LinePrimitiveBuffer);

    constantbufferdesc.ByteWidth = sizeof(FMaterialConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &MaterialConstantBuffer);
    
    constantbufferdesc.ByteWidth = sizeof(FSubMeshConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &SubMeshConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FTextureConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &TextureConstantBufer);

    constantbufferdesc.ByteWidth = sizeof(FFogConstants) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &FogConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FLightingArr) + 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &LightArrConstantBuffer);

    constantbufferdesc.ByteWidth = sizeof(FLitUnlitConstants)+ 0xf & 0xfffffff0;
    Graphics->Device->CreateBuffer(&constantbufferdesc, nullptr, &FlagBuffer);
}

void FRenderer::ReleaseConstantBuffer()
{
    if (ConstantBuffer)
    {
        ConstantBuffer->Release();
        ConstantBuffer = nullptr;
    }

    if (LightArrConstantBuffer)
    {
        LightArrConstantBuffer->Release();
        LightArrConstantBuffer = nullptr;
    }

    if (FlagBuffer)
    {
        FlagBuffer->Release();
        FlagBuffer = nullptr;
    }

    if (MaterialConstantBuffer)
    {
        MaterialConstantBuffer->Release();
        MaterialConstantBuffer = nullptr;
    }

    if (SubMeshConstantBuffer)
    {
        SubMeshConstantBuffer->Release();
        SubMeshConstantBuffer = nullptr;
    }

    if (TextureConstantBufer)
    {
        TextureConstantBufer->Release();
        TextureConstantBufer = nullptr;
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

void FRenderer::UpdateLitUnlitConstant(int isLit) const
{
    if (FlagBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(FlagBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        auto constants = static_cast<FLitUnlitConstants*>(constantbufferMSR.pData); //GPU �޸� ���� ����
        {
            constants->isLit = isLit;
        }
        Graphics->DeviceContext->Unmap(FlagBuffer, 0);
    }
}

void FRenderer::UpdateSubMeshConstant(bool isSelected) const
{
    if (SubMeshConstantBuffer) {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(SubMeshConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FSubMeshConstants* constants = (FSubMeshConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
        {
            constants->isSelectedSubMesh = isSelected;
        }
        Graphics->DeviceContext->Unmap(SubMeshConstantBuffer, 0);
    }
}

void FRenderer::UpdateTextureConstant(float UOffset, float VOffset, float UTiles, float VTiles) const
{
    if (TextureConstantBufer) {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU �� �޸� �ּ� ����
        Graphics->DeviceContext->Map(TextureConstantBufer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR);
        FTextureConstants* constants = (FTextureConstants*)constantbufferMSR.pData; //GPU �޸� ���� ����
        {
            constants->UOffset = UOffset;
            constants->VOffset = VOffset;
            constants->UTiles = UTiles;
            constants->VTiles = VTiles;
        }
        Graphics->DeviceContext->Unmap(TextureConstantBufer, 0);
    }
}

void FRenderer::CreateFontShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/FontVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &FontVertexShader
    );

    hr = D3DCompileFromFile(L"Shaders/FontPixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &FontPixelShader
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &FontInputLayout
    );

    TextureStride = sizeof(FVertexTexture);
    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseFontShader()
{
    if (FontVertexShader)
    {
        FontVertexShader->Release();
        FontVertexShader = nullptr;
    }
    if (FontPixelShader)
    {
        FontPixelShader->Release();
        FontPixelShader = nullptr;
    }
    if (FontInputLayout)
    {
        FontInputLayout->Release();
        FontInputLayout = nullptr;
    }
}

void FRenderer::PrepareFontShader() const
{
    Graphics->DeviceContext->VSSetShader(FontVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(FontPixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(FontInputLayout);
}

void FRenderer::CreateTextureShader()
{
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/TextureVertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", 0, 0, &VertexShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(
        VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), nullptr, &TextureVertexShader
    );

    hr = D3DCompileFromFile(L"Shaders/TexturePixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", 0, 0, &PixelShaderCSO, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(
        PixelShaderCSO->GetBufferPointer(), PixelShaderCSO->GetBufferSize(), nullptr, &TexturePixelShader
    );

    D3D11_INPUT_ELEMENT_DESC layout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    Graphics->Device->CreateInputLayout(
        layout, ARRAYSIZE(layout), VertexShaderCSO->GetBufferPointer(), VertexShaderCSO->GetBufferSize(), &TextureInputLayout
    );

    //�ڷᱸ�� ���� �ʿ�
    TextureStride = sizeof(FVertexTexture);
    VertexShaderCSO->Release();
    PixelShaderCSO->Release();
}

void FRenderer::ReleaseTextureShader()
{
    if (TextureInputLayout)
    {
        TextureInputLayout->Release();
        TextureInputLayout = nullptr;
    }

    if (TexturePixelShader)
    {
        TexturePixelShader->Release();
        TexturePixelShader = nullptr;
    }

    if (TextureVertexShader)
    {
        TextureVertexShader->Release();
        TextureVertexShader = nullptr;
    }
    if (SubUVConstantBuffer)
    {
        SubUVConstantBuffer->Release();
        SubUVConstantBuffer = nullptr;
    }
    if (ConstantBuffer)
    {
        ConstantBuffer->Release();
        ConstantBuffer = nullptr;
    }
}

void FRenderer::PrepareTextureShader() const
{
    Graphics->DeviceContext->VSSetShader(TextureVertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(TexturePixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(TextureInputLayout);

    if (ConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
    }
}

ID3D11Buffer* FRenderer::CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const
{
    // 2. Create a vertex buffer
    D3D11_BUFFER_DESC vertexbufferdesc = {};
    vertexbufferdesc.ByteWidth = byteWidth;
    vertexbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will never be updated 
    vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    //D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

    ID3D11Buffer* vertexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&vertexbufferdesc, nullptr, &vertexBuffer);
    if (FAILED(hr))
    {
        UE_LOG(LogLevel::Warning, "VertexBuffer Creation faild");
    }
    return vertexBuffer;
}

ID3D11Buffer* FRenderer::CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const
{
    D3D11_BUFFER_DESC indexbufferdesc = {};
    indexbufferdesc.Usage = D3D11_USAGE_DYNAMIC;
    indexbufferdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexbufferdesc.ByteWidth = byteWidth;
    indexbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* indexBuffer;

    HRESULT hr = Graphics->Device->CreateBuffer(&indexbufferdesc, nullptr, &indexBuffer);
    if (FAILED(hr))
    {
        return nullptr;
    }
    return indexBuffer;
}

void FRenderer::RenderTexturePrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
    ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    if (numIndices <= 0)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);

    Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
}

//��Ʈ ��ġ������
void FRenderer::RenderTextPrimitive(
    ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
) const
{
    if (!_TextureSRV || !_SamplerState)
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    }
    UINT offset = 0;
    Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);

    // �Է� ���̾ƿ� �� �⺻ ����
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    
    ID3D11DepthStencilState* DepthStateDisable = Graphics->DepthStateDisable;
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStateDisable, 0);

    // ��ο� ȣ�� (6���� �ε��� ���)
    Graphics->DeviceContext->Draw(numVertices, 0);
}


ID3D11Buffer* FRenderer::CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const
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

void FRenderer::UpdateSubUVConstant(float _indexU, float _indexV) const
{
    if (SubUVConstantBuffer)
    {
        D3D11_MAPPED_SUBRESOURCE constantbufferMSR; // GPU�� �޸� �ּ� ����

        Graphics->DeviceContext->Map(SubUVConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
        auto constants = static_cast<FSubUVConstant*>(constantbufferMSR.pData);                               //GPU �޸� ���� ����
        {
            constants->indexU = _indexU;
            constants->indexV = _indexV;
        }
        Graphics->DeviceContext->Unmap(SubUVConstantBuffer, 0); // GPU�� �ٽ� ��밡���ϰ� �����
    }
}

void FRenderer::PrepareSubUVConstant() const
{
    if (SubUVConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &SubUVConstantBuffer);
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &SubUVConstantBuffer);
    }
}

void FRenderer::PrepareLineShader() const
{
    // ���̴��� �Է� ���̾ƿ� ����
    Graphics->DeviceContext->VSSetShader(VertexLineShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelLineShader, nullptr, 0);

    // ��� ���� ���ε�: 
    // - MatrixBuffer�� register(b0)��, Vertex Shader�� ���ε�
    // - GridConstantBuffer�� register(b1)��, Vertex�� Pixel Shader�� ���ε� (�ȼ� ���̴��� �ʿ信 ����)
    if (ConstantBuffer && GridConstantBuffer)
    {
        Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);     // MatrixBuffer (b0)
        Graphics->DeviceContext->VSSetConstantBuffers(1, 1, &GridConstantBuffer); // GridParameters (b1)
        Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &GridConstantBuffer);
        Graphics->DeviceContext->VSSetConstantBuffers(3, 1, &LinePrimitiveBuffer);
        Graphics->DeviceContext->VSSetShaderResources(2, 1, &pBBSRV);
        Graphics->DeviceContext->VSSetShaderResources(3, 1, &pConeSRV);
        Graphics->DeviceContext->VSSetShaderResources(4, 1, &pOBBSRV);
    }
}

void FRenderer::CreateLineShader()
{
    ID3DBlob* VertexShaderLine;
    ID3DBlob* PixelShaderLine;

    HRESULT hr;
    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "VertexShader Error");
    }
    Graphics->Device->CreateVertexShader(VertexShaderLine->GetBufferPointer(), VertexShaderLine->GetBufferSize(), nullptr, &VertexLineShader);

    hr = D3DCompileFromFile(L"Shaders/ShaderLine.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &PixelShaderLine, nullptr);
    if (FAILED(hr))
    {
        Console::GetInstance().AddLog(LogLevel::Warning, "PixelShader Error");
    }
    Graphics->Device->CreatePixelShader(PixelShaderLine->GetBufferPointer(), PixelShaderLine->GetBufferSize(), nullptr, &PixelLineShader);


    VertexShaderLine->Release();
    PixelShaderLine->Release();
}

void FRenderer::ReleaseLineShader() const
{
    if (GridConstantBuffer) GridConstantBuffer->Release();
    if (LinePrimitiveBuffer) LinePrimitiveBuffer->Release();
    if (VertexLineShader) VertexLineShader->Release();
    if (PixelLineShader) PixelLineShader->Release();
}

ID3D11Buffer* FRenderer::CreateStaticVerticesBuffer() const
{
    FSimpleVertex vertices[2]{{0}, {0}};

    D3D11_BUFFER_DESC vbDesc = {};
    vbDesc.Usage = D3D11_USAGE_DEFAULT;
    vbDesc.ByteWidth = sizeof(vertices);
    vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags = 0;
    D3D11_SUBRESOURCE_DATA vbInitData = {};
    vbInitData.pSysMem = vertices;
    ID3D11Buffer* pVertexBuffer = nullptr;
    HRESULT hr = Graphics->Device->CreateBuffer(&vbDesc, &vbInitData, &pVertexBuffer);
    return pVertexBuffer;
}

ID3D11Buffer* FRenderer::CreateBoundingBoxBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ���� ������Ʈ�� ��� DYNAMIC, �׷��� ������ DEFAULT
    bufferDesc.ByteWidth = sizeof(FBoundingBox) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FBoundingBox);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateOBBBuffer(UINT numBoundingBoxes) const
{
    D3D11_BUFFER_DESC bufferDesc;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC; // ���� ������Ʈ�� ��� DYNAMIC, �׷��� ������ DEFAULT
    bufferDesc.ByteWidth = sizeof(FOBB) * numBoundingBoxes;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FOBB);

    ID3D11Buffer* BoundingBoxBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &BoundingBoxBuffer);
    return BoundingBoxBuffer;
}

ID3D11Buffer* FRenderer::CreateConeBuffer(UINT numCones) const
{
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.ByteWidth = sizeof(FCone) * numCones;
    bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    bufferDesc.StructureByteStride = sizeof(FCone);

    ID3D11Buffer* ConeBuffer = nullptr;
    Graphics->Device->CreateBuffer(&bufferDesc, nullptr, &ConeBuffer);
    return ConeBuffer;
}

ID3D11ShaderResourceView* FRenderer::CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;


    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pBBSRV);
    return pBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numBoundingBoxes;
    Graphics->Device->CreateShaderResourceView(pBoundingBoxBuffer, &srvDesc, &pOBBSRV);
    return pOBBSRV;
}

ID3D11ShaderResourceView* FRenderer::CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones)
{
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = DXGI_FORMAT_UNKNOWN; // ����ü ������ ��� UNKNOWN
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    srvDesc.Buffer.ElementOffset = 0;
    srvDesc.Buffer.NumElements = numCones;


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

void FRenderer::ReleaseDefaultPostProcessShader()
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

void FRenderer::CreateFogShader()
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
        FogConstantBuffer->Release();
        FogConstantBuffer = nullptr;
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
