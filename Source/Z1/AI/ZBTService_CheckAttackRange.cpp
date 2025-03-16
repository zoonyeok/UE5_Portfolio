// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/ZBTService_CheckAttackRange.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/ZAIController.h"

void UZBTService_CheckAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{ 
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	//Check distance between AI Pawn and target Actor
	TObjectPtr<UBlackboardComponent> BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackBoardComp))
	{
		return;
	}

	TObjectPtr<AActor> TargetActor = Cast<AActor>(BlackBoardComp->GetValueAsObject("TargetActor"));
	if (!IsValid(TargetActor))
	{
		return;
	}

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
	
	FVector TargetActorLocation = TargetActor->GetActorLocation();
	FVector AILocation = AIPawn->GetActorLocation();
	float Distance = FVector::Distance(TargetActorLocation, AILocation);
 
	bool bWithinRange = Distance < 2000.0f;
	bool bLineOfSightToTarget = false;
	if (bWithinRange)
	{
		bLineOfSightToTarget = AIController->LineOfSightTo(TargetActor);
	}
	bool bResult = bLineOfSightToTarget & bWithinRange;

	BlackBoardComp->SetValueAsBool(AttackRangeKey.SelectedKeyName, bResult);
}
