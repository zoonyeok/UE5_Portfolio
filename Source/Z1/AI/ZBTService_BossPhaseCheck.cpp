// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZBTService_BossPhaseCheck.h"

#include "AIController.h"
#include "ZAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/ZAttributeComponent.h"

void UZBTService_BossPhaseCheck::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	//Check distance between AI Pawn and target Actor
	TObjectPtr<UBlackboardComponent> BlackBoardComp = OwnerComp.GetBlackboardComponent();
	if (!IsValid(BlackBoardComp))
	{
		return;
	}
	// Delegate로 처리 안되나?
	if (TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner())
	{
		if (TObjectPtr<APawn> AIPawn = AIController->GetPawn())
		{
			if (TObjectPtr<AZAICharacter> AICharacter = Cast<AZAICharacter>(AIPawn->GetOwner()))
			{
				if (TObjectPtr<UZAttributeComponent> AttributeComp = AICharacter->GetComponentByClass<
					UZAttributeComponent>())
				{
					int CurrentPhase = AttributeComp->GetCurrentHpPercent() >= 6.0 ? 1 : 2;
					BlackBoardComp->SetValueAsInt(BossPhaseKey.SelectedKeyName, CurrentPhase);
				}
			}
		}
	}
}
