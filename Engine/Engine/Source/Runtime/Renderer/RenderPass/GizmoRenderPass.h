#pragma once
#include "BaseRenderPass.h"
#include "Container/Array.h"

class UMaterial;
struct FMatrix;
class UStaticMeshComponent;
class UGizmoBaseComponent;

class GizmoRenderPass : public BaseRenderPass
{
public:
    explicit GizmoRenderPass(const FString& InShaderName)
        : BaseRenderPass(InShaderName)
    {
    }

    void Prepare(std::shared_ptr<FViewportClient> viewport) override;
    void Execute(std::shared_ptr<FViewportClient> viewport) override;

    void AddRenderObjectsToRenderPass(const ULevel* Level) override;
    void AddGizmoComponent(UGizmoBaseComponent* gizmo) { GizmoComponents.Add(gizmo); }
private:
    TArray<UGizmoBaseComponent*> GizmoComponents;

    static void UpdateMatrixConstants(UGizmoBaseComponent* InGizmoComponent, const FMatrix& InView, const FMatrix& InProjection);
    static void UpdateSubMeshConstants(bool bIsSelectedSubMesh);
    static void UpdateMaterialConstants(const UMaterial* CurrentMaterial);
};
