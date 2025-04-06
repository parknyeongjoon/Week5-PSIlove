#include "IRenderPass.h"

#include "EngineLoop.h"
#include "Components/PrimitiveComponent.h"

extern FEngineLoop GEngineLoop;

void OpaqueRenderPass::Prepare(D3D11_VIEWPORT* viewport)
{
    GEngineLoop.graphicDevice.DeviceContext->ClearRenderTargetView(GEngineLoop.graphicDevice.FrameBufferRTV, GEngineLoop.graphicDevice.ClearColor); // 렌더 타겟 뷰에 저장된 이전 프레임 데이터를 삭제
    GEngineLoop.graphicDevice.DeviceContext->ClearDepthStencilView(GEngineLoop.graphicDevice.DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0); // 깊이 버퍼 초기화 추가
    
    GEngineLoop.graphicDevice.DeviceContext->RSSetState(GEngineLoop.graphicDevice.RasterizerStateSOLID); //레스터 라이저 상태 설정
    GEngineLoop.graphicDevice.DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff); // 블렌뎅 상태 설정, 기본블렌딩 상태임
}

void OpaqueRenderPass::Execute(D3D11_VIEWPORT* viewport)
{
    GEngineLoop.renderer.PrepareShader(ShaderName);
    Prepare(viewport);
    
    for (auto item : Primitives)
    {
        //TODO : Update CBs

        const FVIBuffers* currentVIBuffer =  GEngineLoop.renderer.GetVIBuffer(item->VIBufferName);
        currentVIBuffer->Bind(GEngineLoop.graphicDevice.DeviceContext);

        GEngineLoop.graphicDevice.DeviceContext->DrawIndexed(currentVIBuffer->GetNumIndices(), 0, 0);
    }
}

void OpaqueRenderPass::AddPrimitive(UPrimitiveComponent* Primitive)
{
    Primitives.Add(Primitive);
}
