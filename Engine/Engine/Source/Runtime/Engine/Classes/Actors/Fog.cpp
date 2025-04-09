#include "Fog.h"
#include "Components/HeightFogComponent.h"

AFog::AFog()
{
    UHeightFogComponent* HeightFogComponent = AddComponent<UHeightFogComponent>();
    RootComponent = HeightFogComponent;
}

AFog::~AFog()
{
}

void AFog::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

UObject* AFog::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<AFog>(this);

    // 서브 오브젝트는 깊은 복사로 별도 처리
    Cast<AFog>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
