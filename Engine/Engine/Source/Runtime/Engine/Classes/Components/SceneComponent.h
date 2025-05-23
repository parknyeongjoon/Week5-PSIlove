#pragma once
#include "ActorComponent.h"
#include "Math/Quat.h"
#include "UObject/ObjectMacros.h"

class USceneComponent : public UActorComponent
{
    DECLARE_CLASS(USceneComponent, UActorComponent)

public:
    USceneComponent();
    virtual ~USceneComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    virtual FVector GetForwardVector();
    virtual FVector GetRightVector();
    virtual FVector GetUpVector();
    void AddLocation(FVector _added);
    void AddRotation(FVector _added);
    void AddScale(FVector _added);

protected:
    FVector RelativeLocation;
    FVector RelativeRotation;
    FQuat QuatRotation;
    FVector RelativeScale3D;

    USceneComponent* AttachParent = nullptr;
    TArray<USceneComponent*> AttachChildren;

public:
    virtual FVector GetWorldRotation();
    FVector GetWorldScale();
    FVector GetWorldLocation();
    FVector GetLocalRotation();
    FQuat GetQuat() const { return QuatRotation; }

    FVector GetLocalScale() const { return RelativeScale3D; }
    FVector GetLocalLocation() const { return RelativeLocation; }

    void SetLocation(FVector _newLoc) { RelativeLocation = _newLoc; }
    virtual void SetRotation(FVector _newRot);
    void SetRotation(FQuat _newRot) { QuatRotation = _newRot; }
    void SetScale(FVector _newScale) {
        RelativeScale3D = _newScale; 
    }
    void SetupAttachment(USceneComponent* InParent);

    virtual void DuplicateSubObjects() override;
    virtual UObject* Duplicate() override;

private:

public:
    const TArray<USceneComponent*>& GetAttachChildren() const; // ���������� ���� children�� ��ȯ
    void GetChildrenComponents(TArray<USceneComponent*>& Children) const; // ��ͷ� �Ʒ��� ��� children ��ȯ
    USceneComponent* GetAttachParent() const;
    void GetParentComponents(TArray<USceneComponent*>& Parents) const; // ��ͷ� root���� ��ȯ

    void SetupAttachment(TArray<USceneComponent*>& Children); // �ڽ� ���� ���� ����
    void ClearAttachment(){AttachChildren.Empty();}

    // �θ��ڽ� ���踦 ������ ���ϴ� �� �Լ� ����
    bool AttachToComponent(USceneComponent* Parent); // �θ� ���� ���������� false
};
