#include "HeightFogComponent.h"

UHeightFogComponent::UHeightFogComponent()
{
    SetType(StaticClass()->GetName());
}

UHeightFogComponent::~UHeightFogComponent()
{
}

void UHeightFogComponent::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
}

UObject* UHeightFogComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UHeightFogComponent>(this);

    Cast<UHeightFogComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UHeightFogComponent::InitializeComponent()
{
    Super::InitializeComponent();
    CreatePostProcessBuffer();
}

void UHeightFogComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UHeightFogComponent::CreatePostProcessBuffer()
{
    FScreenVertex vertices[4] =
    {
        { FVector4(-1.0f,  1.0f, 0.0f, 1.0f), 0.0f, 0.0f },
        { FVector4(1.0f,  1.0f, 0.0f, 1.0f), 1.0f, 0.0f },
        { FVector4(1.0f, -1.0f, 0.0f, 1.0f), 1.0f, 1.0f },
        { FVector4(-1.0f, -1.0f, 0.0f, 1.0f), 0.0f, 1.0f }
    };
    
    uint32 indices[6] =
    {
        0, 1, 2, // 첫 번째 삼각형
        0, 2, 3  // 두 번째 삼각형
    };

    ID3D11Buffer* vertexBuffer = GEngineLoop.renderer.CreateDynamicVertexBuffer<FScreenVertex>(vertices, 4);
    FEngineLoop::renderer.AddOrSetVertexBuffer(TEXT("FogQuad"), vertexBuffer, sizeof(FScreenVertex), 4);
    
    ID3D11Buffer* indexBuffer = GEngineLoop.renderer.CreateIndexBuffer(indices, 6);
    FEngineLoop::renderer.AddOrSetIndexBuffer(TEXT("FogQuad"), indexBuffer, 6);
    
    VIBufferName = TEXT("FogQuad");
}
