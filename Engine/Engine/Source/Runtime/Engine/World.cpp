#include "World.h"
#include "Level.h"
#include "GameFramework/Actor.h"
void UWorld::DuplicateObject(const UObject* SourceObject)
{
    Super::DuplicateObject(SourceObject);

    UWorld* sourceWorld = Cast<UWorld>(SourceObject);
    Level = sourceWorld->GetLevel();
}

void UWorld::DuplicateSubObjects()
{
    Level = Level->Duplicate<ULevel>();
}
