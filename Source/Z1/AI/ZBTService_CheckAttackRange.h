// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ZBTService_CheckAttackRange.generated.h"

/**
 *
 */
UCLASS()
class Z1_API UZBTService_CheckAttackRange : public UBTService
{
	GENERATED_BODY()

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, Category="AI")
	FBlackboardKeySelector AttackRangeKey;
};
