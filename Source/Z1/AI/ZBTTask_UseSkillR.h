// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "ZBTTask_UseSkillR.generated.h"

/**
 * 
 */
class AZAICharacter;
UCLASS(Blueprintable)
class Z1_API UZBTTask_UseSkillR : public UBTTaskNode
{
	GENERATED_BODY()

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void PhaseOneSkillExecute(const TObjectPtr<AZAICharacter>& AICharacter);
	void PhaseTwoSkillExecute(const TObjectPtr<AZAICharacter>& AICharacter);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> PhaseOneSkill;

	//VFX
	//SOUND
	//AnimNotify
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> PhaseTwoSkill;
};
