#include "Engine/Source/Runtime/CoreUObject/UObject/Object.h"

#include "UClass.h"
#include "UObjectHash.h"


UClass* UObject::StaticClass()
{
    static UClass ClassInfo{TEXT("UObject"), sizeof(UObject), alignof(UObject), nullptr};
    return &ClassInfo;
}

UObject* UObject::Duplicate()
{
    // 새 객체 생성
    UObject* NewObject = new UObject(*this); // 얕은 복사 수행

    // 서브 오브젝트는 깊은 복사로 별도 처리
    NewObject->DuplicateSubObjects();

    return NewObject;
}

void UObject::DuplicateSubObjects()
{
    // TODO: 
}

void UObject::DuplicateObjects()
{
    // TODO:
}

UObject::UObject()
    : UUID(0)
    // TODO: Object를 생성할 때 직접 설정하기
    , InternalIndex(std::numeric_limits<uint32>::max())
    , NamePrivate("None")
{
}

bool UObject::IsA(const UClass* SomeBase) const
{
    const UClass* ThisClass = GetClass();
    return ThisClass->IsChildOf(SomeBase);
}
