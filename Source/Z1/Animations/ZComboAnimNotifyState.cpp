// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/ZComboAnimNotifyState.h"
#include "Components/ZActionComponent.h"
#include "Z1Character.h"
#include "ZAnimInstance.h"
#include "Actions/ZComboAction.h"

class AZ1Character;

UZComboAnimNotifyState::UZComboAnimNotifyState()
{
}

void UZComboAnimNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	if (MeshComp)
	{
		if (AZ1Character* Character = Cast<AZ1Character>(MeshComp->GetOwner()))
		{
			if (UZActionComponent* ActionComponent = Character->GetComponentByClass<UZActionComponent>())
			{
				if (UZComboAction* ComboAction = Cast<UZComboAction>(ActionComponent->FindActionByTag(ActionTag)))
				{
					ComboAction->SetNextComboSegment(NextLightComboName, NextHeavyComboName);
				}
			}
		}
	}
}

void UZComboAnimNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                       const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	//NextCombo Montage_section 실행

	if (MeshComp)
	{
		if (AZ1Character* Character = Cast<AZ1Character>(MeshComp->GetOwner()))
		{
			if (UZActionComponent* ActionComponent = Character->GetComponentByClass<UZActionComponent>())
			{
				if (UZComboAction* ComboAction = Cast<UZComboAction>(ActionComponent->FindActionByTag(ActionTag)))
				{
					ComboAction->SetDefaultCombos();
				}
			}
		}
	}
}
