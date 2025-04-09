#pragma once
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"

class URotationMovementComponent;
class UProjectileMovementComponent;
class UTextRenderComponent;
class UTextBillboardComponent;
class UBillboardComponent;
class UStaticMeshComponent;

class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;


private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const;
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const;

    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp);
    
    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderCreateMaterialView();

    /* Text Settings */
    void RenderForTextRender(UTextRenderComponent* TextRenderComp);
    void RenderForTextBillboard(UTextBillboardComponent* TextBillboardComp);

    /* Billboard Settings */
    void RenderForBillboard(UBillboardComponent* BillboardComp);
    /* Movement Settings */
    void RenderForRotation(URotationMovementComponent* RotationMovementComponent);
    void RenderForProjectile(UProjectileMovementComponent* ProjectileMovementComponent);
private:
    float Width = 0, Height = 0;
    FVector Location = FVector(0, 0, 0);
    FVector Rotation = FVector(0, 0, 0);
    FVector Scale = FVector(0, 0, 0);

    /* RotationMovementProperty */
    FVector RotationRate = FVector(0, 0, 0);
    /* ProjecttileMovementProperty */
    FVector Velocity = FVector(0,0,0);
    FVector MaxSpeed = FVector(0,0,0);
    FVector Acceleration = FVector(0,0,0);
    
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial;
};
