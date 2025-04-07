#include "StaticMeshRenderPass.h"

#include "EngineLoop.h"
#include "ViewportClient.h"
#include "Components/PrimitiveComponent.h"
#include "D3D11RHI/GPUBuffer/FVIBuffers.h"

extern FEngineLoop GEngineLoop;

void StaticMeshRenderPass::Prepare(FViewportClient* viewport)
{
    GEngineLoop.graphicDevice.DeviceContext->ClearRenderTargetView(GEngineLoop.graphicDevice.FrameBufferRTV.Get(), GEngineLoop.graphicDevice.ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    GEngineLoop.graphicDevice.DeviceContext->ClearDepthStencilView(GEngineLoop.graphicDevice.DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가

    GEngineLoop.graphicDevice.DeviceContext->OMSetRenderTargets(1, GEngineLoop.graphicDevice.FrameBufferRTV.GetAddressOf(), GEngineLoop.graphicDevice.DepthStencilView.Get());
    
    GEngineLoop.graphicDevice.DeviceContext->RSSetState(GEngineLoop.graphicDevice.RasterizerStateSOLID.Get()); //레스터 라이저 상태 설정
    GEngineLoop.graphicDevice.DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
    GEngineLoop.graphicDevice.DeviceContext->RSSetViewports(1, viewport->GetD3DViewport());
    GEngineLoop.graphicDevice.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    GEngineLoop.renderer.PrepareShader(ShaderName);
    Prepare(viewport);
}

void StaticMeshRenderPass::Execute(FViewportClient* viewport)
{
    // 쉐이더 내에서 한 번만 Update되어야하는 CB
    
    for (const auto item : Primitives)
    {
        // TODO : Update CBs
        // Object마다 Update되어야하는 CB

        const std::shared_ptr<FVIBuffers> currentVIBuffer =  GEngineLoop.renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(GEngineLoop.graphicDevice.DeviceContext.Get());

        GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0, 0);
    }

    Primitives.Empty();
}

void StaticMeshRenderPass::AddPrimitive(UPrimitiveComponent* Primitive)
{
    Primitives.Add(Primitive);
}