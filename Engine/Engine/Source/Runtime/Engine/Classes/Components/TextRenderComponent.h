#pragma once
#include "PrimitiveComponent.h"

class UTextRenderComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UTextRenderComponent, UPrimitiveComponent)

public:
    UTextRenderComponent();
    virtual ~UTextRenderComponent() override;
    
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    void ClearText();
    void SetText(FWString _text);
    FWString GetText() { return text; }
    void SetRowColumnCount(int32 InRowCount, int32 InColumnCount);
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;

    void SetTexture(FWString _fileName);
    
    ID3D11Buffer* vertexTextBuffer;
    TArray<FVertexTexture> vertexTextureArr;
    UINT numTextVertices;

    float finalIndexU = 0.0f;
    float finalIndexV = 0.0f;

    std::shared_ptr<FTexture> Texture;
    
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
    void CreateTextTextureVertexBuffer(const TArray<FVertexTexture>& _vertex, UINT byteWidth);
};
