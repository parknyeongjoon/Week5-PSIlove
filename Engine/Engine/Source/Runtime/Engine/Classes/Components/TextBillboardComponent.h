#pragma once
#include "BillboardComponent.h"

class UTextBillboardComponent : public UBillboardComponent
{
    DECLARE_CLASS(UTextBillboardComponent, UBillboardComponent)

public:
    UTextBillboardComponent();
    virtual ~UTextBillboardComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    void ClearText();
    void SetText(FWString _text);
    FWString GetText() { return text; }
    void SetRowColumnCount(int32 InRowCount, int32 InColumnCount);
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    
    TArray<FVertexTexture> vertexTextureArr;
    UINT numTextVertices;
protected:
    FWString text;

    TArray<FVector> quad;

    const int quadSize = 2;

    int RowCount;
    int ColumnCount;

    float quadWidth = 2.0f;
    float quadHeight = 2.0f;

    void setStartUV(char alphabet, float& outStartU, float& outStartV);
    void setStartUV(wchar_t hangul, float& outStartU, float& outStartV);
};
