#include "CubeComp.h"
#include "Math/JungleMath.h"
#include "Level.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "UObject/ObjectFactory.h"
#include "PropertyEditor/ShowFlags.h"

#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"

#include "Engine/FLoaderOBJ.h"

UCubeComp::UCubeComp()
{
    SetType(StaticClass()->GetName());
    AABB.max = { 1,1,1 };
    AABB.min = { -1,-1,-1 };

}

UCubeComp::~UCubeComp()
{
}

void UCubeComp::DuplicateSubObjects()
{
}

void UCubeComp::Duplicate(const UObject* SourceObject)
{
    Super::Duplicate(SourceObject);

    // 없음.
}

void UCubeComp::InitializeComponent()
{
    Super::InitializeComponent();

    FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    SetStaticMesh(FManagerOBJ::GetStaticMesh(L"helloBlender.obj"));
}

void UCubeComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}