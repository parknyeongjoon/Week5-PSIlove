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
