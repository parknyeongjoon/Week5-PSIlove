#include "World.h"
#include "Level.h"
#include "GameFramework/Actor.h"
void UWorld::Duplicate(const UObject* SourceObject)
{
    Super::Duplicate(SourceObject);

    UWorld* sourceWorld = Cast<UWorld>(SourceObject);
    Level = sourceWorld->GetLevel();
}

void UWorld::DuplicateSubObjects()
{
    Level = Level->Duplicate<ULevel>();
}
