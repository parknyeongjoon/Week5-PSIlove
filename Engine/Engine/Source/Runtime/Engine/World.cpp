#include "World.h"
#include "Level.h"
#include "GameFramework/Actor.h"

void UWorld::WorldTick(float DeltaTime)
{
    for (auto* Actor : Level->GetActors())
    {
        if (Actor && Actor->IsActorTickEnable())
        {
            Actor->Tick(DeltaTime);
        }
    }
}

void UWorld::DuplicateObject(const UObject* SourceObject)
{
    UWorld* sourceWorld = Cast<UWorld>(SourceObject);
    Level = sourceWorld->GetLevel();
}

void UWorld::DuplicateSubObjects()
{
    Level = Level->Duplicate<ULevel>();
}
