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
}

UCubeComp::~UCubeComp()
{
}

void UCubeComp::DuplicateSubObjects()
{
    Super::DuplicateSubObjects();
}

UObject* UCubeComp::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<UCubeComp>(this);

    Cast<UCubeComp>(NewObject)->DuplicateSubObjects();
    return NewObject;
}

void UCubeComp::InitializeComponent()
{
    Super::InitializeComponent();

    UStaticMesh* staticMesh = FManagerOBJ::CreateStaticMesh("Assets/helloBlender.obj");
    SetStaticMesh(staticMesh);
}

void UCubeComp::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}