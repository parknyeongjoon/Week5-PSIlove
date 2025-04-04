#include "SphereComp.h"
#include "Engine/Source/Runtime/Core/Math/JungleMath.h"
#include "Engine/Source/Runtime/Engine/Level.h"
#include "Engine/Source/Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/PrimitiveBatch.h"


USphereComp::USphereComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = {1, 1, 1};
    AABB.min = {-1, -1, -1};
}

USphereComp::~USphereComp()
{
}

void USphereComp::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
}

UObject* USphereComp::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<USphereComp>(this);

    Cast<USphereComp>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void USphereComp::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}