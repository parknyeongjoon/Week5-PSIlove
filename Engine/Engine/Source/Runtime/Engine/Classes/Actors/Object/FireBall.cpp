#include "FireBall.h"

#include "Components/StaticMeshComponent.h"
#include "Components/LightComponent.h"
#include "Engine/FLoaderOBJ.h"

AFireBall::AFireBall()
{
    FManagerOBJ::CreateStaticMesh("Assets/Fireball.obj");
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Fireball.obj"));
    AddComponent<ULightComponent>();
}

void AFireBall::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

UObject* AFireBall::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<AFireBall>(this);

    // 서브 오브젝트는 깊은 복사로 별도 처리
    Cast<AFireBall>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
