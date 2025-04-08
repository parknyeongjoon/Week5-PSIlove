#pragma once
#include "BaseRenderPass.h"
#include "Container/Array.h"

class UMaterial;
class USkySphereComponent;
struct FMatrix;
class UStaticMeshComponent;

class StaticMeshRenderPass: public BaseRenderPass
{
public:
    StaticMeshRenderPass(const FString& InShaderName)
        : BaseRenderPass(InShaderName) {}
    
    virtual void Prepare(std::shared_ptr<FViewportClient> viewport) override;
    virtual void Execute(std::shared_ptr<FViewportClient> viewport) override;

    void AddRenderObjectsToRenderPass(const ULevel* Level) override;
    void AddStaticMesh(UStaticMeshComponent* InStaticMesh) { StaticMesheComponents.Add(InStaticMesh); }
private:
    static void UpdateMatrixConstants(UStaticMeshComponent* InStaticMeshComponent, const FMatrix& InView, const FMatrix& InProjection);
    static void UpdateSkySphereTextureConstants(const USkySphereComponent* InSkySphereComponent);
    static void UpdateSubMeshConstants(bool bIsSelectedSubMesh);
    static void UpdateMaterialConstants(const UMaterial* CurrentMaterial);
    
    TArray<UStaticMeshComponent*> StaticMesheComponents;
};
