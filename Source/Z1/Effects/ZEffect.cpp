// Fill out your copyright notice in the Description page of Project Settings.


#include "ZEffect.h"


void UZEffect::Initialize(UZEffectComponent* InOwningComponent, const FZEffectContext& Context, double ServerNow)
{
	OwningComponent = InOwningComponent;
	SourceActor = Context.SourceActor;
	DamageCauser = Context.DamageCauser;

	EffectHandle = FGuid::NewGuid();
	StackCount = 1;
	bIsActive = true;

	StartServerTime = ServerNow;
	NextExecutionServerTime = ServerNow + Period;
	EndServerTime = (DurationPolicy == EZEffectDurationPolicy::HasDuration) ? (ServerNow + Duration) : ServerNow;
}

void UZEffect::OnApplied()
{
}

void UZEffect::ExecuteEffect()
{
}

void UZEffect::OnStackChanged(int32 OldStackCount)
{
}

void UZEffect::OnRemoved(EZEffectRemovalReason Reason)
{
	bIsActive = false;
}

double UZEffect::GetRemainingTime(double ServerNow) const
{
	if (DurationPolicy != EZEffectDurationPolicy::HasDuration)
	{
		return -1.0;
	}

	return FMath::Max(0.0, EndServerTime - ServerNow);
}
