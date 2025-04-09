#include "FinalRenderPass.h"

#include "Define.h"
#include "EngineLoop.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"
#include "UnrealEd/EditorViewportClient.h"

extern FEngineLoop GEngineLoop;

FinalRenderPass::FinalRenderPass(const FString& InName)
    : BaseRenderPass(InName)
{
    CreatePostProcessBuffer();
}

void FinalRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewport)
{
    BaseRenderPass::Prepare(InViewport);
    
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    Graphics.SwapRTV();
    Graphics.DeviceContext->OMSetDepthStencilState(nullptr, 0);
    Graphics.DeviceContext->OMSetRenderTargets(1, &Graphics.FrameBufferRTV, nullptr); // 렌더 타겟 설정(백버퍼를 가르킴)
}

void FinalRenderPass::Execute(std::shared_ptr<FViewportClient> InViewport)
{
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;
    FRenderer& Renderer = GEngineLoop.renderer;

    std::shared_ptr<FEditorViewportClient> activeViewport = std::dynamic_pointer_cast<FEditorViewportClient>(InViewport);

    UpdatePostProcessQuadVertexBufferUpdate(activeViewport);
    
    // SceneColor + Depth SRV 바인딩
    ID3D11ShaderResourceView* SRVs[2] = { Graphics.GetReadSRV(), Graphics.pingpongDepthSRV[0] };
    Graphics.DeviceContext->PSSetShaderResources(0, 2, SRVs);

    const std::shared_ptr<FVIBuffers> currentVIBuffer = Renderer.GetVIBuffer(VIBufferName);
    currentVIBuffer->Bind(Graphics.DeviceContext);
    // 풀스크린 쿼드 그리기
    Graphics.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0, 0);

    // SRV 해제 (다음 패스를 위한 정리)
    ID3D11ShaderResourceView* nullSRV[2] = { nullptr, nullptr };
    Graphics.DeviceContext->PSSetShaderResources(0, 2, nullSRV);

    // // Sampler 해제
    // ID3D11SamplerState* nullSamplers[1] = { nullptr };
    // Graphics.DeviceContext->PSSetSamplers(0, 1, nullSamplers);
}

void FinalRenderPass::AddRenderObjectsToRenderPass(const ULevel* InLevel)
{
}

void FinalRenderPass::CreatePostProcessBuffer()
{
    FRenderer& Renderer = GEngineLoop.renderer;

    FScreenVertex vertices[4] =
    {
        { FVector4(-1.0f,  1.0f, 0.0f, 1.0f), 0.0f, 0.0f },
        { FVector4(1.0f,  1.0f, 0.0f, 1.0f), 1.0f, 0.0f },
        { FVector4(1.0f, -1.0f, 0.0f, 1.0f), 1.0f, 1.0f },
        { FVector4(-1.0f, -1.0f, 0.0f, 1.0f), 0.0f, 1.0f }
    };

    const uint32 indices[6] =
    {
        0, 1, 2, // 첫 번째 삼각형
        0, 2, 3  // 두 번째 삼각형
    };

    ID3D11Buffer* vertexBuffer = Renderer.CreateDynamicVertexBuffer<FScreenVertex>(vertices, 4);
    Renderer.AddOrSetVertexBuffer(TEXT("FinalQuad"), vertexBuffer, sizeof(FScreenVertex), 4);
    
    ID3D11Buffer* indexBuffer = Renderer.CreateIndexBuffer(indices, 6);
    Renderer.AddOrSetIndexBuffer(TEXT("FinalQuad"), indexBuffer, 6);
    
    VIBufferName = TEXT("FinalQuad");
}

void FinalRenderPass::UpdatePostProcessQuadVertexBufferUpdate(const std::shared_ptr<FEditorViewportClient>& InActiveViewport) const
{
    FRenderer& Renderer = GEngineLoop.renderer;
    FGraphicsDevice& Graphics = GEngineLoop.graphicDevice;

    const float screenWidth = static_cast<float>(Graphics.screenWidth);
    const float screenHeight = static_cast<float>(Graphics.screenHeight);

    D3D11_VIEWPORT* activeD3DViewport = InActiveViewport->GetD3DViewport();
    const float uvMinX = activeD3DViewport->TopLeftX / screenWidth;
    const float uvMinY = activeD3DViewport->TopLeftY / screenHeight;
    const float uvMaxX = (activeD3DViewport->TopLeftX + activeD3DViewport->Width) / screenWidth;
    const float uvMaxY = (activeD3DViewport->TopLeftY + activeD3DViewport->Height) / screenHeight;

    FScreenVertex vertices[4] = {
        { FVector4(-1.0f, 1.0f, 0.0f, 1.0f), uvMinX, uvMinY }, // top-left
        { FVector4(1.0f, 1.0f, 0.0f, 1.0f), uvMaxX, uvMinY }, // top-right
        { FVector4(1.0f, -1.0f, 0.0f, 1.0f), uvMaxX, uvMaxY }, // bottom-right
        { FVector4(-1.0f, -1.0f, 0.0f, 1.0f), uvMinX, uvMaxY }  // bottom-left
    };

    ID3D11Buffer* VertexBuffer = Renderer.GetVIBuffer(VIBufferName)->GetVertexBuffer();
    Renderer.UpdateVertexBuffer(VertexBuffer, &vertices, 4);
}
