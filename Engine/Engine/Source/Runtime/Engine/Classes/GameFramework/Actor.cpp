#include "Actor.h"

#include "Level.h"
#include "Components/StaticMeshComponent.h"

void AActor::BeginPlay()
{
    // TODO: 나중에 삭제를 Pending으로 하던가 해서 복사비용 줄이기
    const auto CopyComponents = OwnedComponents;
    for (UActorComponent* Comp : CopyComponents)
    {
        Comp->BeginPlay();
    }
}

void AActor::Tick(float DeltaTime)
{
    // TODO: 임시로 Actor에서 Tick 돌리기
    // TODO: 나중에 삭제를 Pending으로 하던가 해서 복사비용 줄이기
    const auto CopyComponents = OwnedComponents;
    for (UActorComponent* Comp : CopyComponents)
    {
        Comp->TickComponent(DeltaTime);
    }
}

void AActor::Destroyed()
{
    // Actor가 제거되었을 때 호출하는 EndPlay
    EndPlay(EEndPlayReason::Destroyed);
}

void AActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 본인이 소유하고 있는 모든 컴포넌트의 EndPlay 호출
    for (UActorComponent* Component : GetComponents())
    {
        if (Component->HasBegunPlay())
        {
            Component->EndPlay(EndPlayReason);
        }
    }
    UninitializeComponents();
}

bool AActor::Destroy()
{
    if (!IsActorBeingDestroyed())
    {
        if (ULevel* level = GetLevel())
        {
            level->DestroyActor(this);
            bActorIsBeingDestroyed = true;
        }
    }

    return IsActorBeingDestroyed();
}

void AActor::RemoveOwnedComponent(UActorComponent* Component)
{
    OwnedComponents.Remove(Component);
}

void AActor::InitializeComponents()
{
    TSet<UActorComponent*> Components = GetComponents();
    for (UActorComponent* ActorComp : Components)
    {
        if (ActorComp->bAutoActive && !ActorComp->IsActive())
        {
            ActorComp->Activate();
        }

        if (!ActorComp->HasBeenInitialized())
        {
            ActorComp->InitializeComponent();
        }
    }
}

void AActor::UninitializeComponents()
{
    TSet<UActorComponent*> Components = GetComponents();
    for (UActorComponent* ActorComp : Components)
    {
        if (ActorComp->HasBeenInitialized())
        {
            ActorComp->UninitializeComponent();
        }
    }
}

bool AActor::SetRootComponent(USceneComponent* NewRootComponent)
{
    if (NewRootComponent == nullptr || NewRootComponent->GetOwner() == this)
    {
        if (RootComponent != NewRootComponent)
        {
            USceneComponent* OldRootComponent = RootComponent;
            RootComponent = NewRootComponent;

            OldRootComponent->SetupAttachment(RootComponent);
        }
        return true;
    }
    return false;
}

bool AActor::SetActorLocation(const FVector& NewLocation)
{
    if (RootComponent)
    {
        RootComponent->SetLocation(NewLocation);
        return true;
    }
    return false;
}

bool AActor::SetActorRotation(const FVector& NewRotation)
{
    if (RootComponent)
    {
        RootComponent->SetRotation(NewRotation);
        return true;
    }
    return false;
}

bool AActor::SetActorScale(const FVector& NewScale)
{
    if (RootComponent)
    {
        RootComponent->SetScale(NewScale);
        return true;
    }
    return false;
}

void AActor::DuplicateSubObjects()
{
    if (OwnedComponents.Num() == 0)
        return;

    OwnedComponents.Remove(RootComponent);
    
    RootComponent = Cast<USceneComponent>(RootComponent->Duplicate());
    // RootComponent 아래있는 모든 scenecomponent가 생성됨
    // tree구조 완성되었지만
    // owner는 그대로

    TSet<USceneComponent*> NewSceneComponents;
    RootComponent->GetChildrenComponents(NewSceneComponents);
    NewSceneComponents.Add(RootComponent);
    // Owner 바꿔주기
    for (auto& s : NewSceneComponents)
    {
        Cast<UActorComponent>(s)->Owner = this;
        OwnedComponents.Add(Cast<UActorComponent>(s));
    }
}

UObject* AActor::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<AActor>(this);

    // 서브 오브젝트는 깊은 복사로 별도 처리
    Cast<AActor>(NewObject)->DuplicateSubObjects();
    return NewObject;
}
