// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZBTTask_UseSkillQ.h"

#include "AIController.h"
#include "ZAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"


EBTNodeResult::Type UZBTTask_UseSkillQ::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner();
	if (!IsValid(AIController))
	{
		return EBTNodeResult::Failed;
	}

	TObjectPtr<AZAICharacter> AICharacter = Cast<AZAICharacter>(AIController->GetPawn());
	if (!IsValid(AICharacter))
	{
		return EBTNodeResult::Failed;
	}

	TObjectPtr<UBlackboardComponent> BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackboardComponent))
	{
		return EBTNodeResult::Failed;
	}

	const int BossPhase = BlackboardComponent->GetValueAsInt("BossPhase");
	if (BossPhase > 2)
	{
		return EBTNodeResult::Failed;
	}

	FName MuzzleSocket = AICharacter->GetWeaponSocketName();
	FVector SocketLocation = AICharacter->GetMesh()->GetSocketLocation(MuzzleSocket);
	FRotator MuzzleRotation = AICharacter->GetMesh()->GetSocketRotation(MuzzleSocket);

	if (BossPhase == 1)
	{
		PhaseOneSkillExecute(AICharacter);
	}
	else
	{
		PhaseTwoSkillExecute(AICharacter);
	}
	
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}

void UZBTTask_UseSkillQ::PhaseOneSkillExecute(const TObjectPtr<AZAICharacter>& AICharacter)
{
	//AnimMontage실행
	if (IsValid(AICharacter))
	{
		AICharacter->PlayAnimMontage(PhaseOneSkill);
	}
}

void UZBTTask_UseSkillQ::PhaseTwoSkillExecute(const TObjectPtr<AZAICharacter>& AICharacter)
{
	//AnimMontage실행
	if (IsValid(AICharacter))
	{
		AICharacter->PlayAnimMontage(PhaseTwoSkill);
	}
}
