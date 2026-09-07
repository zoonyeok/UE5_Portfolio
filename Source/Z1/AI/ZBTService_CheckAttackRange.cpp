// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/ZBTService_CheckAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/ZAIController.h"
#include "Perception/PawnSensingComponent.h"

void UZBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{ 
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	// 1. 블랙보드 컴포넌트 가져오기
	TObjectPtr<UBlackboardComponent> BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComp))
	{
		return;
	}

	// 2. 타겟 유효성 체크
	TObjectPtr<AActor> TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
	if (!IsValid(TargetActor))
	{
		return;
	}

	// 3. AIController 및 AI Pawn 가져오기
	TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return;
	}

	TObjectPtr<APawn> AIPawn = AIController->GetPawn();
	if (!IsValid(AIPawn))
	{
		return;
	}

	// 4. PawnSensingComponent 가져오기
	TObjectPtr<UPawnSensingComponent> SensingComp = AIPawn->FindComponentByClass<UPawnSensingComponent>();
	if (!IsValid(SensingComp))
	{
		return;
	}

	// 5. 거리 계산
	RangedAttackDistance = SensingComp->SightRadius;

	const FVector AILocation = AIPawn->GetActorLocation();
	const FVector TargetLocation = TargetActor->GetActorLocation();
	const float DistanceToTarget = FVector::Distance(TargetLocation, AILocation);

	const bool bWithinRanged = DistanceToTarget < RangedAttackDistance;
	const bool bWithinMelee = DistanceToTarget < MeleeAttackDistance;

	// 6. 시야각(FOV) 계산
	bool bIsInFOV = false;
	if (bWithinRanged || bWithinMelee)
	{
		const FVector DirectionToTarget = (TargetLocation - AILocation).GetSafeNormal();
		const FVector ForwardDirection = AIPawn->GetActorForwardVector();

		const float Dot = FVector::DotProduct(ForwardDirection, DirectionToTarget);
		const float CosHalfFOV = FMath::Cos(FMath::DegreesToRadians(SensingComp->GetPeripheralVisionAngle() / 2.f));

		bIsInFOV = Dot >= CosHalfFOV;
	}

	// 7. Line of Sight 체크
	bool bHasLineOfSight = false;
	if (bIsInFOV && (bWithinRanged || bWithinMelee))
	{
		bHasLineOfSight = AIController->LineOfSightTo(TargetActor);
	}

	// 8. 최종 결과 계산
	const bool bCanRangedAttack = bHasLineOfSight && bWithinRanged && bIsInFOV;
	const bool bCanMeleeAttack = bHasLineOfSight && bWithinMelee && bIsInFOV;

	// 9. 블랙보드에 값 설정
	BlackboardComp->SetValueAsBool(RangedAttackRangeKey.SelectedKeyName, bCanRangedAttack);
	BlackboardComp->SetValueAsBool(MeleeAttackRangeKey.SelectedKeyName, bCanMeleeAttack);
}
