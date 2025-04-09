#pragma once
#include "BaseRenderPass.h"
#include "Container/Array.h"

struct FObjMaterialInfo;
class UMaterial;
class USkySphereComponent;
struct FMatrix;
class UStaticMeshComponent;

class StaticMeshRenderPass: public BaseRenderPass
{
public:
    StaticMeshRenderPass(const FString& InShaderName)
        : BaseRenderPass(InShaderName) {}
    
    virtual void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    virtual void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void AddRenderObjectsToRenderPass(const ULevel* InLevel) override;
    void AddStaticMesh(UStaticMeshComponent* InStaticMesh) { StaticMesheComponents.Add(InStaticMesh); }
private:
    static void UpdateMatrixConstants(UStaticMeshComponent* InStaticMeshComponent, const FMatrix& InView, const FMatrix& InProjection);
    static void UpdateSkySphereTextureConstants(const USkySphereComponent* InSkySphereComponent);
    static void UpdateSubMeshConstants(bool bIsSelectedSubMesh);
    static void UpdateMaterialConstants(const FObjMaterialInfo& MaterialInfo);
    
    TArray<UStaticMeshComponent*> StaticMesheComponents;
};
