#include "World.h"
#include "Level.h"
#include "GameFramework/Actor.h"
UObject* UWorld::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UWorld>(this);

    // 서브 오브젝트는 깊은 복사로 별도 처리
    Cast<UWorld>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UWorld::DuplicateSubObjects()
{
    Level = Cast<ULevel>(Level->Duplicate());
}
