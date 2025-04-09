#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#define _TCHAR_DEFINED
#include <d3d11.h>
#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Set.h"

class UPrimitiveComponent;
class ULightComponent;
class ULevel;
class FGraphicsDevice;
class UMaterial;
struct FStaticMaterial;
class UObject;
class FEditorViewportClient;
class UBillboardComponent;
class UStaticMeshComponent;
class UGizmoBaseComponent;
class UHeightFogComponent;
class FRenderer 
{

private:
    float litFlag = 0;
public:
    // GPU 기본 연결
    FGraphicsDevice* Graphics;

    // 셰이더 관련
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11VertexShader* QuadShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11PixelShader* LightingPixelShader = nullptr;
    ID3D11PixelShader* DepthPixelShader = nullptr;
    ID3D11InputLayout* InputLayout = nullptr;

    // ConstantBuffer 그룹
    ID3D11Buffer* ConstantBuffer = nullptr;
    ID3D11Buffer* LightArrConstantBuffer = nullptr;
    ID3D11Buffer* FlagBuffer = nullptr;
    ID3D11Buffer* MaterialConstantBuffer = nullptr;
    ID3D11Buffer* SubMeshConstantBuffer = nullptr;
    ID3D11Buffer* TextureConstantBufer = nullptr;

    // VertexBuffer 그룹 (FullScreenQaud)
    ID3D11Buffer* FullScreenQuadVertexBuffer = nullptr;
    ID3D11Buffer* FullScreenQuadIndexBuffer = nullptr;

    FLighting lightingData;

    uint32 Stride;
    uint32 Stride2;

public:
    void Initialize(FGraphicsDevice* graphics);
   
    void PrepareShader() const;
    void PrepareGizmoShader() const;
    void PrepareLightingShader() const;

    //Render
    void RenderPrimitive(OBJ::FStaticMeshRenderData* renderData, TArray<FStaticMaterial*> materials, TArray<UMaterial*> overrideMaterial, int selectedSubMeshIndex) const;
   
    void RenderTexturedModelPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* InTextureSRV, ID3D11SamplerState* InSamplerState) const;
    //Release
    void Release();
    void ReleaseShader();
    void ReleaseBuffer(ID3D11Buffer*& Buffer) const;
    void ReleaseConstantBuffer();

    void ResetVertexShader() const;
    void ResetPixelShader() const;
    void CreateShader();

    void SetVertexShader(const FWString& filename, const FString& funcname, const FString& version);
    void SetPixelShader(const FWString& filename, const FString& funcname, const FString& version);
    
    void ChangeViewMode(EViewModeIndex evi) const;
    
    // CreateBuffer
    void CreateConstantBuffer();
    ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateVertexBuffer(const TArray<FVertexSimple>& vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(uint32* indices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexBuffer(const TArray<uint32>& indices, UINT byteWidth) const;

    void PrepareDepthShader() const;

    // update
    void UpdateConstant(const FMatrix& Model, const FMatrix& ViewProjection, const FMatrix& NormalMatrix, bool IsSelected) const;
    void UpdateLightBuffer(TArray<ULightComponent*> lightComponents) const;
    void UpdateMaterial(const FObjMaterialInfo& MaterialInfo) const;
    void UpdateLitUnlitConstant(int isLit) const;
    void UpdateSubMeshConstant(bool isSelected) const;
    void UpdateTextureConstant(float UOffset, float VOffset, float UTiles, float VTiles) const;

public://텍스쳐용 기능 추가
    ID3D11VertexShader* TextureVertexShader = nullptr;
    ID3D11PixelShader* TexturePixelShader = nullptr;
    ID3D11InputLayout* TextureInputLayout = nullptr;

    ID3D11VertexShader* FontVertexShader = nullptr;
    ID3D11PixelShader* FontPixelShader = nullptr;
    ID3D11InputLayout* FontInputLayout = nullptr;

    uint32 TextureStride;
    struct FSubUVConstant
    {
        float indexU;
        float indexV;
    };
    ID3D11Buffer* SubUVConstantBuffer = nullptr;

public:
    void CreateFontShader();
    void ReleaseFontShader();

    void PrepareFontShader() const;
    
    void CreateTextureShader();
    void ReleaseTextureShader();
    
    void PrepareTextureShader() const;
    ID3D11Buffer* CreateVertexTextureBuffer(FVertexTexture* vertices, UINT byteWidth) const;
    ID3D11Buffer* CreateIndexTextureBuffer(uint32* indices, UINT byteWidth) const;
    void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11Buffer* pIndexBuffer, UINT numIndices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;
    void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
        ID3D11ShaderResourceView* _TextureSRV,
        ID3D11SamplerState* _SamplerState) const;
    ID3D11Buffer* CreateVertexBuffer(FVertexTexture* vertices, UINT byteWidth) const;

    void UpdateSubUVConstant(float _indexU, float _indexV) const;
    void PrepareSubUVConstant() const;


public: // line shader
    void PrepareLineShader() const;
    void CreateLineShader();
    void ReleaseLineShader() const;
    void RenderBatch(const FGridParameters& gridParam, ID3D11Buffer* pVertexBuffer, int boundingBoxCount, int coneCount, int coneSegmentCount, int obbCount) const;
    void SetRenderObj(ULevel* Level);
    void UpdateGridConstantBuffer(const FGridParameters& gridParams) const;
    void UpdateLinePrimitveCountBuffer(int numBoundingBoxes, int numCones) const;
    ID3D11Buffer* CreateStaticVerticesBuffer() const;
    ID3D11Buffer* CreateBoundingBoxBuffer(UINT numBoundingBoxes) const;
    ID3D11Buffer* CreateOBBBuffer(UINT numBoundingBoxes) const;
    ID3D11Buffer* CreateConeBuffer(UINT numCones) const;
    ID3D11ShaderResourceView* CreateBoundingBoxSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes);
    ID3D11ShaderResourceView* CreateOBBSRV(ID3D11Buffer* pBoundingBoxBuffer, UINT numBoundingBoxes);
    ID3D11ShaderResourceView* CreateConeSRV(ID3D11Buffer* pConeBuffer, UINT numCones);

    void UpdateBoundingBoxBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FBoundingBox>& BoundingBoxes, int numBoundingBoxes) const;
    void UpdateOBBBuffer(ID3D11Buffer* pBoundingBoxBuffer, const TArray<FOBB>& BoundingBoxes, int numBoundingBoxes) const;
    void UpdateConesBuffer(ID3D11Buffer* pConeBuffer, const TArray<FCone>& Cones, int numCones) const;

    //Render Pass Demo
    void ClearRenderArr();
    void Render(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderStaticMeshes(ULevel* Level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderGizmos(const ULevel* Level, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderLighting(ULevel* Level, std::shared_ptr<FEditorViewportClient>& ActiveViewport) const;
    void RenderDepthScene(ULevel* Level, std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    void RenderBillboards(ULevel* Level,std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderTexts(ULevel* Level,std::shared_ptr<FEditorViewportClient> ActiveViewport);
    
private:
    TArray<UStaticMeshComponent*> StaticMeshObjs;
    TArray<UGizmoBaseComponent*> GizmoObjs;
    TArray<UPrimitiveComponent*> TextObjs;
    TArray<UBillboardComponent*> BillboardObjs;
    TArray<ULightComponent*> LightObjs;
    TArray<UHeightFogComponent*> HeightFogObjs;

public:
    ID3D11VertexShader* VertexLineShader = nullptr;
    ID3D11PixelShader* PixelLineShader = nullptr;
    ID3D11Buffer* GridConstantBuffer = nullptr;
    ID3D11Buffer* LinePrimitiveBuffer = nullptr;
    ID3D11ShaderResourceView* pBBSRV = nullptr;
    ID3D11ShaderResourceView* pConeSRV = nullptr;
    ID3D11ShaderResourceView* pOBBSRV = nullptr;
    // default postprocess
public:
    ID3D11PixelShader* PostProcessPixelShader = nullptr;
    void CreateDefaultPostProcessShader();
    void ReleaseDefaultPostProcessShader();
    void PrepareDefaultPostProcessShader() const;
    // fogshader
public:
    ID3D11PixelShader* FogPixelShader = nullptr;
    ID3D11Buffer* FogConstantBuffer = nullptr;
    ID3D11ShaderResourceView* FogSRV = nullptr;

    void CreateFogShader();
    void ReleaseFogShader();
    void PrepareFogShader() const;
    void UpdateFogConstant(UHeightFogComponent* FogComponent, const FMatrix& InvProjectionMatrix, const FMatrix& InvViewMatrix, const FVector CameraPosition);
    void RenderFog(ULevel* level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    void RenderFinal(ULevel* level, std::shared_ptr<FEditorViewportClient> ActiveViewport);
};

