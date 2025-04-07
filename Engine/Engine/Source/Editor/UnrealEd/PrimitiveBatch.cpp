#include "PrimitiveBatch.h"
#include "EngineLoop.h"
#include "UnrealEd/EditorViewportClient.h"
extern FEngineLoop GEngineLoop;

UPrimitiveBatch::UPrimitiveBatch()
{
    GenerateGrid(5, 5000);
}

UPrimitiveBatch::~UPrimitiveBatch()
{
}


void UPrimitiveBatch::GenerateGrid(float spacing, int gridCount)
{
    GridParam.GridSpacing = spacing;
    GridParam.GridCount = gridCount;
    GridParam.GridOrigin = { 0,0,0 };
}

void UPrimitiveBatch::RenderBatch(const FMatrix& View, const FMatrix& Projection)
{
    //FEngineLoop::renderer.PrepareLineShader();
    FEngineLoop::renderer.PrepareShader(TEXT("Line"));

    //InitializeVertexBuffer();

    FMatrix Model = FMatrix::Identity;
    FMatrix MVP = Model * View * Projection;
    FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
    //FEngineLoop::renderer.UpdateConstant(MVP, NormalMatrix, FVector4(0,0,0,0), false);
    //FEngineLoop::renderer.UpdateGridConstantBuffer(GridParam);

    UpdateBoundingBoxResources();
    //UpdateConeResources();
    //UpdateOBBResources();
    int boundingBoxSize = static_cast<int>(BoundingBoxes.Num());
    int coneSize = static_cast<int>(Cones.Num());
    int obbSize = static_cast<int>(OrientedBoundingBoxes.Num());
    FEngineLoop::renderer.UpdateLinePrimitveCountBuffer(boundingBoxSize, coneSize);
    //FEngineLoop::renderer.RenderBatch(GridParam, pVertexBuffer, boundingBoxSize, coneSize, ConeSegmentCount, obbSize);
    BoundingBoxes.Empty();
    Cones.Empty();
    OrientedBoundingBoxes.Empty();
    FEngineLoop::renderer.PrepareShader(TEXT("StaticMesh"));
}

void UPrimitiveBatch::UpdateBoundingBoxResources()
{
    if (BoundingBoxes.Num() > allocatedBoundingBoxCapacity)
    {
        allocatedBoundingBoxCapacity = BoundingBoxes.Num();

        ReleaseBoundingBoxResources();

        Microsoft::WRL::ComPtr<ID3D11Buffer> SB = nullptr;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SBSRV = nullptr;
        SB = FEngineLoop::renderer.CreateStructuredBuffer<FBoundingBox>(static_cast<UINT>(allocatedBoundingBoxCapacity));
        SBSRV = FEngineLoop::renderer.CreateBufferSRV(SB, static_cast<UINT>(allocatedBoundingBoxCapacity));

        FEngineLoop::renderer.AddOrSetStructuredBuffer(TEXT("BoundingBox"), SB);
        FEngineLoop::renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("BoundingBox"), SBSRV);
    }

    const Microsoft::WRL::ComPtr<ID3D11Buffer> SB = FEngineLoop::renderer.GetStructuredBuffer(TEXT("BoundingBox"));
    const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> SBSRV = FEngineLoop::renderer.GetStructuredBufferShaderResourceView(TEXT("BoundingBox"));
    if (SB != nullptr && SBSRV != nullptr)
    {
        FEngineLoop::renderer.UpdateStructuredBuffer(SB, BoundingBoxes);
    }
}

void UPrimitiveBatch::ReleaseBoundingBoxResources()
{

}

//void UPrimitiveBatch::UpdateConeResources()
//{
//    if (Cones.Num() > allocatedConeCapacity)
//    {
//        allocatedConeCapacity = Cones.Num();
//
//        ReleaseConeResources();
//
//        ID3D11Buffer* SB = nullptr;
//        ID3D11ShaderResourceView* SBSRV = nullptr;
//        SB = FEngineLoop::renderer.CreateStructuredBuffer<FCone>(static_cast<UINT>(allocatedConeCapacity));
//        SBSRV = FEngineLoop::renderer.CreateBufferSRV(pConesBuffer, static_cast<UINT>(allocatedConeCapacity));
//        
//        FEngineLoop::renderer.AddOrSetStructuredBuffer(TEXT("Cone"), SB);
//        FEngineLoop::renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("Cone"), SBSRV);
//    }
//
//    ID3D11Buffer* SB = FEngineLoop::renderer.GetStructuredBuffer(TEXT("Cone"));
//    const ID3D11ShaderResourceView* SBSRV = FEngineLoop::renderer.GetStructuredBufferShaderResourceView(TEXT("Cone"));
//    if (SB != nullptr && SBSRV != nullptr)
//    {
//        FEngineLoop::renderer.UpdateStructuredBuffer(SB, BoundingBoxes);
//    }
//}
//
//void UPrimitiveBatch::ReleaseConeResources()
//{
//    if (pConesBuffer) pConesBuffer->Release();
//    if (pConesSRV) pConesSRV->Release();
//}
//
//void UPrimitiveBatch::UpdateOBBResources()
//{
//    if (OrientedBoundingBoxes.Num() > allocatedOBBCapacity)
//    {
//        allocatedOBBCapacity = OrientedBoundingBoxes.Num();
//
//        ReleaseOBBResources();
//
//        ID3D11Buffer* SB = nullptr;
//        ID3D11ShaderResourceView* SBSRV = nullptr;
//        SB = FEngineLoop::renderer.CreateStructuredBuffer<FOBB>(static_cast<UINT>(allocatedOBBCapacity));
//        SBSRV = FEngineLoop::renderer.CreateBufferSRV(pOBBBuffer, static_cast<UINT>(allocatedOBBCapacity));
//
//        FEngineLoop::renderer.AddOrSetStructuredBuffer(TEXT("OBB"), SB);
//        FEngineLoop::renderer.AddOrSetStructuredBufferShaderResourceView(TEXT("OBB"), SBSRV);
//    }
//
//    ID3D11Buffer* SB = FEngineLoop::renderer.GetStructuredBuffer(TEXT("OBB"));
//    const ID3D11ShaderResourceView* SBSRV = FEngineLoop::renderer.GetStructuredBufferShaderResourceView(TEXT("OBB"));
//    if (SB != nullptr && SBSRV != nullptr)
//    {
//        FEngineLoop::renderer.UpdateStructuredBuffer(SB, BoundingBoxes);
//    }
//}

void UPrimitiveBatch::RenderAABB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
{
    FVector localVertices[8] = {
         { localAABB.min.x, localAABB.min.y, localAABB.min.z },
         { localAABB.max.x, localAABB.min.y, localAABB.min.z },
         { localAABB.min.x, localAABB.max.y, localAABB.min.z },
         { localAABB.max.x, localAABB.max.y, localAABB.min.z },
         { localAABB.min.x, localAABB.min.y, localAABB.max.z },
         { localAABB.max.x, localAABB.min.y, localAABB.max.z },
         { localAABB.min.x, localAABB.max.y, localAABB.max.z },
         { localAABB.max.x, localAABB.max.y, localAABB.max.z }
    };

    FVector worldVertices[8];
    worldVertices[0] = center + FMatrix::TransformVector(localVertices[0], modelMatrix);

    FVector min = worldVertices[0], max = worldVertices[0];

    // 첫 번째 값을 제외한 나머지 버텍스를 변환하고 min/max 계산
    for (int i = 1; i < 8; ++i)
    {
        worldVertices[i] = center + FMatrix::TransformVector(localVertices[i], modelMatrix);

        min.x = (worldVertices[i].x < min.x) ? worldVertices[i].x : min.x;
        min.y = (worldVertices[i].y < min.y) ? worldVertices[i].y : min.y;
        min.z = (worldVertices[i].z < min.z) ? worldVertices[i].z : min.z;

        max.x = (worldVertices[i].x > max.x) ? worldVertices[i].x : max.x;
        max.y = (worldVertices[i].y > max.y) ? worldVertices[i].y : max.y;
        max.z = (worldVertices[i].z > max.z) ? worldVertices[i].z : max.z;
    }
    FBoundingBox BoundingBox;
    BoundingBox.min = min;
    BoundingBox.max = max;
    BoundingBoxes.Add(BoundingBox);
}
void UPrimitiveBatch::RenderOBB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
{
    // 1) 로컬 AABB의 8개 꼭짓점
    FVector localVertices[8] =
    {
        { localAABB.min.x, localAABB.min.y, localAABB.min.z },
        { localAABB.max.x, localAABB.min.y, localAABB.min.z },
        { localAABB.min.x, localAABB.max.y, localAABB.min.z },
        { localAABB.max.x, localAABB.max.y, localAABB.min.z },
        { localAABB.min.x, localAABB.min.y, localAABB.max.z },
        { localAABB.max.x, localAABB.min.y, localAABB.max.z },
        { localAABB.min.x, localAABB.max.y, localAABB.max.z },
        { localAABB.max.x, localAABB.max.y, localAABB.max.z }
    };

    FOBB faceBB;
    for (int32 i = 0; i < 8; ++i) {
        // 모델 매트릭스로 점을 변환 후, center를 더해준다.
        faceBB.corners[i] =  center + FMatrix::TransformVector(localVertices[i], modelMatrix);
    }

    OrientedBoundingBoxes.Add(faceBB);

}

void UPrimitiveBatch::AddCone(const FVector& center, float radius, float height, int segments, const FVector4& color, const FMatrix& modelMatrix)
{
    ConeSegmentCount = segments;
    FVector localApex = FVector(0, 0, 0);
    FCone cone;
    cone.ConeApex = center + FMatrix::TransformVector(localApex, modelMatrix);
    FVector localBaseCenter = FVector(height, 0, 0);
    cone.ConeBaseCenter = center + FMatrix::TransformVector(localBaseCenter, modelMatrix);
    cone.ConeRadius = radius;
    cone.ConeHeight = height;
    cone.Color = color;
    cone.ConeSegmentCount = ConeSegmentCount;
    Cones.Add(cone);
}

