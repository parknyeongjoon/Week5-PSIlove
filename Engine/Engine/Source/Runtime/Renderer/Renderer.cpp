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
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    Graphics->CreateVertexShader(TEXT("StaticMeshVertexShader.hlsl"), &VSBlob_StaticMesh, &VertexShader);
    Graphics->CreatePixelShader(TEXT("StaticMeshPixelShader.hlsl"), &PSBlob_StaticMesh, &PixelShader);

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

void FRenderer::CreateDefaultPostProcessShader()
{    
    ID3DBlob* VertexShaderCSO;
    ID3DBlob* PixelShaderCSO;

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
    FFlagConstants flag;
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        bIsLit = true;
        flag.IsLit = true;
        // UpdateConstnatBuffer<FFlagConstants>(ConstantBuffers[TEXT("FFlagConstants")], &flag);
        SetCurrentRasterizerState(ERasterizerState::SolidBack);
        break;
    case EViewModeIndex::VMI_Wireframe:
        SetCurrentRasterizerState(ERasterizerState::WireFrame);
        break;
    case EViewModeIndex::VMI_Unlit:
        bIsLit = false;
        flag.IsLit = false;
        // UpdateConstnatBuffer<FFlagConstants>(ConstantBuffers[TEXT("FFlagConstants")], &flag);
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

void FRenderer::Render(ULevel* InLevel, std::shared_ptr<FEditorViewportClient> InActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, InActiveViewport->GetD3DViewport());
    ChangeViewMode(InActiveViewport->GetViewMode());

    // for (const auto renderPass : RenderPasses)
    // {
    //     renderPass->Prepare(ActiveViewport);
    //     renderPass->Execute(ActiveViewport);
    // }
    
    staticMeshRenderPass->Prepare(InActiveViewport);
    staticMeshRenderPass->Execute(InActiveViewport);

    fontRenderPass->Prepare(InActiveViewport);
    fontRenderPass->Execute(InActiveViewport);
    
    billboardRenderPass->Prepare(InActiveViewport);
    billboardRenderPass->Execute(InActiveViewport);
    
    if (InActiveViewport->ViewMode == VMI_Lit)
    {
        lightingRenderPass->Prepare(InActiveViewport);
        lightingRenderPass->Execute(InActiveViewport);
    }

    if (InActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_HeightFog))
    {
        fogRenderPass->Prepare(InActiveViewport);
        fogRenderPass->Execute(InActiveViewport);
    }

    lineBatchRenderPass->Prepare(InActiveViewport);
    lineBatchRenderPass->Execute(InActiveViewport);

    if (InLevel->GetSelectedActor() != nullptr)
    {
        gizmoRenderPass->Prepare(InActiveViewport);
        gizmoRenderPass->Execute(InActiveViewport);
    }

    finalRenderPass->Prepare(InActiveViewport);
    finalRenderPass->Execute(InActiveViewport);
}