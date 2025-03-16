// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ZRollAction.h"
#include "GameplayTagContainer.h"
#include "Z1Character.h"
#include "Animations/ZAnimInstance.h"
#include "ZGameplayTags.h"
#include "Components/ZActionComponent.h"

UZRollAction::UZRollAction()
{
	ActionName = "RollAction";
	bAutoStart = false;

    // 태그 추가
	ActivationTag = ZGameplayTags::Action_Movement_Rolling;

    BlockedTags.AddTag(ZGameplayTags::State_Movement_Jumping);
	BlockedTags.AddTag(ZGameplayTags::State_Movement_Sprinting);

    GrantsTags.AddTag(ZGameplayTags::State_Movement_Rolling);
}

void UZRollAction::StartAction(AActor* Instigator)
{
    Super::StartAction(Instigator);

	UZAnimInstance* AnimInstance = GetOwningAnimInstance();
	if (!IsValid(AnimInstance) || AnimInstance->Montage_IsPlaying(RollMontage))
	{
		return;
	}

	AnimInstance->Montage_Play(RollMontage);

	// Montage 종료 시점에 호출될 델리게이트 바인딩
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UZRollAction::OnRollMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, RollMontage);
}

void UZRollAction::StopAction(AActor* Instigator)
{
    Super::StopAction(Instigator);
}

void UZRollAction::OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == RollMontage && !bInterrupted)
	{
		AActor* Instigator = GetOwningComponent()->GetOwner();
		if (IsValid(Instigator))
		{
			StopAction(Instigator);
		}
	}
}
