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
    if (pVertexBuffer) {
        pVertexBuffer->Release();
        pVertexBuffer = nullptr;
    }
    ReleaseOBBResources();
    ReleaseBoundingBoxResources();
    ReleaseConeResources();
}

void UPrimitiveBatch::GenerateGrid(float spacing, int gridCount)
{
    GridParam.GridSpacing = spacing;
    GridParam.GridCount = gridCount;
    GridParam.GridOrigin = { 0,0,0 };
}

void UPrimitiveBatch::AddAABB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
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
void UPrimitiveBatch::AddOBB(const FBoundingBox& localAABB, const FVector& center, const FMatrix& modelMatrix)
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

void UPrimitiveBatch::AddCone(const FVector& center, const float radius, const float height, const int segments, const FVector4& color, const FMatrix& modelMatrix)
{
    ConeSegmentCount = segments;
    const FVector localApex = FVector(0, 0, 0);
    FCone cone;
    cone.ConeApex = center + FMatrix::TransformVector(localApex, modelMatrix);
    const FVector localBaseCenter = FVector(height, 0, 0);
    cone.ConeBaseCenter = center + FMatrix::TransformVector(localBaseCenter, modelMatrix);
    cone.ConeRadius = radius;
    cone.ConeHeight = height;
    cone.Color = color;
    cone.ConeSegmentCount = ConeSegmentCount;
    Cones.Add(cone);
}

