#include "Actor.h"

#include "Level.h"

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
    if (OwnedComponents.Num() > 0)
    {
        for (const auto& Comp : OwnedComponents) 
        {
            if (Comp == Cast<UActorComponent>(RootComponent)) 
            {
                USceneComponent* NewRootComp = Comp->Duplicate<USceneComponent>();
                RootComponent = NewRootComp;
                RootComponent->Owner = this;
            }
        }
        TSet<UActorComponent*> NewOwnedComps;
        for (const auto& Comp : OwnedComponents) 
        {
            UActorComponent* NewComp = Comp->Duplicate<UActorComponent>();
            NewComp->Owner = this;
            if (USceneComponent* NewSceneComp = Cast<USceneComponent>(Comp))
            {
                if (NewSceneComp != RootComponent)
                {
                    NewSceneComp->SetupAttachment(RootComponent);
                }
            }
            NewComp->InitializeComponent();
            NewOwnedComps.Add(NewComp);
        }
        OwnedComponents = NewOwnedComps;
    }
}

void AActor::DuplicateObject(const UObject* SourceObject)
{
    if (AActor* SourceActor = Cast<AActor>(SourceObject)) 
    {
        bActorIsBeingDestroyed = false;
        SetActorLabel(SourceActor->GetActorLabel());
        RootComponent = SourceActor->GetRootComponent();
        OwnedComponents = SourceActor->GetComponents();
    }
}

AActor* AActor::DuplicateAndAdd()
{
    AActor* NewActor = Duplicate<AActor>();
    GetLevel()->AddActor(NewActor);
    return NewActor;
}
