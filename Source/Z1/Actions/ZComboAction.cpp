// Fill out your copyright notice in the Description page of Project Settings.

#include "Actions/ZComboAction.h"
#include "GameFramework/Character.h"
#include "TimerManager.h"
#include "Animations/ZAnimInstance.h"
#include "Player/ZComboActionData.h"

DEFINE_LOG_CATEGORY(LogComboAction);

UZComboAction::UZComboAction()
{
	DefaultLightComboAttack = "Light01";
	DefaultHeavyComboAttack = "Heavy01";
	SetDefaultCombos();
}

void UZComboAction::StartAction(AActor* Instigator)
{
	Super::StartAction(Instigator);
	
	UZAnimInstance* AnimInstance = GetOwningAnimInstance();
	if (AnimInstance == nullptr) return;

	if (!AnimInstance->Montage_IsPlaying(ComboMontage))
	{
		AnimInstance->Montage_Play(ComboMontage);
	}
	
	AnimInstance->Montage_JumpToSection(LightComboAttack);
	
}

void UZComboAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);
}

void UZComboAction::SetDefaultCombos()
{
	LightComboAttack = DefaultLightComboAttack;
	HeavyComboAttack = DefaultHeavyComboAttack;
}

void UZComboAction::SetNextComboSegment(FName LightCombo, FName HeavyCombo)
{
	LightComboAttack = LightCombo;
	HeavyComboAttack = HeavyCombo;
}
