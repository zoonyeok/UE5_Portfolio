// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ZDodgeAction.h"
#include "Animations/ZAnimInstance.h"
#include "Z1Character.h"

UZDodgeAction::UZDodgeAction()
{
}

void UZDodgeAction::StartAction(AActor* Instigator)
{
	Super::StartAction(Instigator);

	if (UZAnimInstance* AnimInstance = GetOwningAnimInstance())
	{
		AnimInstance->SetDodgeState(true);
	}
}

void UZDodgeAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);

}
