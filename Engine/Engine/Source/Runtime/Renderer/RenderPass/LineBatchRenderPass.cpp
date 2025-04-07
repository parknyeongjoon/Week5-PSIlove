#include "LineBatchRenderPass.h"

#include "EngineLoop.h"
#include "ViewportClient.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/Casts.h"

extern FEngineLoop GEngineLoop;

LineBatchRenderPass::LineBatchRenderPass(const FString& InShaderName)
    : IRenderPass(InShaderName)
{
    FSimpleVertex vertices[2]{{0}, {0}};
    Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer = FEngineLoop::renderer.CreateStaticVertexBuffer<FSimpleVertex>(vertices, 2);
    FEngineLoop::renderer.AddOrSetVertexBuffer(TEXT("Line"), pVertexBuffer, sizeof(FSimpleVertex));
}

void LineBatchRenderPass::Prepare(FViewportClient* viewport)
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

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> BoundingBoxSRV = GEngineLoop.renderer.GetStructuredBufferShaderResourceView(TEXT("BoundingBox"));
    GEngineLoop.graphicDevice.DeviceContext->VSSetShaderResources(2, 1, BoundingBoxSRV.GetAddressOf());

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ConeSRV = GEngineLoop.renderer.GetStructuredBufferShaderResourceView(TEXT("Cone"));
    GEngineLoop.graphicDevice.DeviceContext->VSSetShaderResources(3, 1, ConeSRV.GetAddressOf());

    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> OBBSRV = GEngineLoop.renderer.GetStructuredBufferShaderResourceView(TEXT("OBB"));
    GEngineLoop.graphicDevice.DeviceContext->VSSetShaderResources(4, 1, OBBSRV.GetAddressOf());
}

void LineBatchRenderPass::Execute(FViewportClient* viewport)
{
    FEditorViewportClient* curEditorViewportClient = dynamic_cast<FEditorViewportClient*>(viewport);

    // 쉐이더 내에서 한 번만 Update되어야하는 CB
    const FMatrix Model = FMatrix::Identity;
    FMatrix MVP;
    if (curEditorViewportClient != nullptr)
        MVP = Model * curEditorViewportClient->GetViewMatrix() * curEditorViewportClient->GetProjectionMatrix();
    const FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));

    FMatrixConstants MatrixConstants;
    MatrixConstants.MVP = MVP;
    MatrixConstants.MInverseTranspose = NormalMatrix;
    MatrixConstants.UUID = FVector4(0,0,0,0);
    MatrixConstants.isSelected = false;
    
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FMatrixConstants")), &MatrixConstants);

    const FGridParametersData GridParameters = PrimitiveBatch->GetGridParameters();
    GEngineLoop.renderer.UpdateConstant(GEngineLoop.renderer.GetConstantBuffer(TEXT("FGridParametersData")), &GridParameters);
    // GEngineLoop.renderer.UpdateStructuredBuffer()
}
