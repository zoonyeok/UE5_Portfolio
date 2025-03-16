// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ZJumpAction.h"
#include "ZGameplayTags.h"
#include "GameFramework/Character.h"
#include "Animations/ZAnimInstance.h"
#include "Components/ZActionComponent.h"

UZJumpAction::UZJumpAction()
{
	ActionName = "JumpAction";
	bAutoStart = false;
	
	// 태그 추가
	ActivationTag = ZGameplayTags::Action_Movement_Jumping;

	BlockedTags.AddTag(ZGameplayTags::State_Movement_Rolling);
	BlockedTags.AddTag(ZGameplayTags::State_Movement_Sprinting);

	GrantsTags.AddTag(ZGameplayTags::State_Movement_Jumping);
}

void UZJumpAction::StartAction(AActor* Instigator)
{
	Super::StartAction(Instigator);

	UZAnimInstance* AnimInstance = GetOwningAnimInstance();
	if (!IsValid(AnimInstance) || AnimInstance->Montage_IsPlaying(JumpMontage))
	{
		return;
	}

	/*ACharacter* Character = GetCharacter();
	if (IsValid(Character))
	{
		Character->Jump();
	}*/

	AnimInstance->Montage_Play(JumpMontage);

	// Montage 종료 시점에 호출될 델리게이트 바인딩
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UZJumpAction::OnJumpMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, JumpMontage);
}

void UZJumpAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);

	ACharacter* Character = GetCharacter();
	if (IsValid(Character))
	{
		Character->StopJumping();
	}
}

void UZJumpAction::OnJumpMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == JumpMontage && !bInterrupted)
	{
		AActor* Instigator = GetOwningComponent()->GetOwner();
		if (IsValid(Instigator))
		{
			StopAction(Instigator);
		}
	}
}
