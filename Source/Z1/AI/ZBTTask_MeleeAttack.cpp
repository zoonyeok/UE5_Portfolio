// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZBTTask_MeleeAttack.h"

#include "AIController.h"
#include "ZAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"


EBTNodeResult::Type UZBTTask_MeleeAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return EBTNodeResult::Failed;
	}

	TObjectPtr<AZAICharacter> MyPawn = Cast<AZAICharacter>(AIController->GetPawn());
	if (!IsValid(MyPawn))
	{
		return EBTNodeResult::Failed;
	}
	
	TObjectPtr<UBlackboardComponent> BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComponent))
	{
		return EBTNodeResult::Failed;
	}

	TObjectPtr<AActor> TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject("TargetActor"));
	if (!IsValid(TargetActor))
	{
		return EBTNodeResult::Failed;
	}
	
	// // ✅ 발사 방향 계산
	// FVector DirectionToTarget = (TargetActor->GetActorLocation() - MyPawn->GetActorLocation()).GetSafeNormal();
	// FRotator Rotator = DirectionToTarget.Rotation();
	
	if (USkeletalMeshComponent* MeshComp = MyPawn->GetMesh())
	{
		 if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		 {
		 	AnimInstance->Montage_Play(MeleeAttackMontage);
			return EBTNodeResult::Succeeded;
		 }
	}


	return EBTNodeResult::Failed;
	//return Super::ExecuteTask(OwnerComp, NodeMemory);
}
