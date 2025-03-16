// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ZWeaponTraceNotifyState.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogWeaponTrace, Log, All);

class AZ1Character;
/**
 * 애니메이션 실행 중 Bone Transform을 추적하여 무기 궤적을 계산
 */
UCLASS()
class Z1_API UZWeaponTraceNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	UPROPERTY(EditAnywhere, Category="Debug")
	float Radius = 10.f;

	UPROPERTY(EditAnywhere, Category="Debug")
	int32 AnimSegmentCount = 10;

	UPROPERTY(EditAnywhere, Category="Debug")
	int32 SegmentCount = 5;

	UPROPERTY(EditAnywhere, Category="Debug")
	float LifeTime = 1.0f;

	UPROPERTY(EditAnywhere, Category="Debug")
	TEnumAsByte<ECollisionChannel> CollisionChannel;
	
protected:
	TPair<FTransform, FTransform> PrevTransform;

	UPROPERTY()
	TObjectPtr<AZ1Character> Owner;

	UPROPERTY()
	TSet<AActor*> AlreadyHitActors;

	UPROPERTY(EditAnywhere, Category="Weapon")
	FName WeaponBoneName;
	
	UPROPERTY()
	const UAnimMontage* CurrentAnimMontage;

	bool bHasPreviousData = false;

	void GenerateCollisionMesh(const TPair<FTransform, FTransform>& CurrentTransform);
	void CheckTriangleCollision(const FVector& A, const FVector& B, const FVector& C);
	void TriangleIntersectingChecking(const TArray<FOverlapResult>& OverlapResults);

	// TODO
	void ClearData();
};
