// Fill out your copyright notice in the Description page of Project Settings.


#include "ZEffectComponent.h"
#include "Engine/World.h"


UZEffectComponent::UZEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	// 활성 효과가 있을 때만 켠다 (ApplyEffect/UpdateEffects에서 토글)
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UZEffectComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZEffectComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (const UWorld* World = GetWorld())
	{
		UpdateEffects(World->GetTimeSeconds());
	}
}

FZEffectApplyResult UZEffectComponent::ApplyEffect(TSubclassOf<UZEffect> EffectClass, const FZEffectContext& Context)
{
	FZEffectApplyResult Result;

	if (!CanApplyEffect(EffectClass, Context))
	{
		return Result;
	}

	const UZEffect* ClassDefaults = GetDefault<UZEffect>(EffectClass);
	UWorld* World = GetWorld();
	const double ServerNow = World ? World->GetTimeSeconds() : 0.0;

	if (UZEffect* ExistingEffect = FindStackableEffect(EffectClass, Context))
	{
		const int32 OldStackCount = ExistingEffect->StackCount;

		if (ClassDefaults->StackPolicy == EZEffectStackPolicy::IncreaseStack)
		{
			ExistingEffect->StackCount = FMath::Clamp(ExistingEffect->StackCount + 1, 1, ClassDefaults->MaxStacks);
		}

		if (ClassDefaults->bRefreshDurationOnReapply && ClassDefaults->DurationPolicy == EZEffectDurationPolicy::HasDuration)
		{
			ExistingEffect->EndServerTime = ServerNow + ClassDefaults->Duration;
		}

		if (ClassDefaults->bResetPeriodOnReapply && ClassDefaults->Period > 0.f)
		{
			ExistingEffect->NextExecutionServerTime = ServerNow + ClassDefaults->Period;
		}

		Result.bSuccess = true;
		Result.EffectHandle = ExistingEffect->GetEffectHandle();

		if (ExistingEffect->StackCount != OldStackCount)
		{
			ExistingEffect->OnStackChanged(OldStackCount);
			Result.ResultType = EZEffectApplicationResult::StackIncreased;
		}
		else
		{
			Result.ResultType = EZEffectApplicationResult::DurationRefreshed;
		}

		OnEffectChanged.Broadcast(ExistingEffect);
		return Result;
	}

	UZEffect* NewEffect = NewObject<UZEffect>(this, EffectClass);
	NewEffect->Initialize(this, Context, ServerNow);

	if (NewEffect->DurationPolicy == EZEffectDurationPolicy::Instant)
	{
		NewEffect->ExecuteEffect();

		Result.bSuccess = true;
		Result.EffectHandle = NewEffect->GetEffectHandle();
		Result.ResultType = EZEffectApplicationResult::ExecutedInstant;
		return Result;
	}

	if (bIsUpdatingEffects)
	{
		PendingApplications.Add(NewEffect);
	}
	else
	{
		ActiveEffects.Add(NewEffect);
	}

	GrantEffectTags(NewEffect);
	NewEffect->OnApplied();

	if (NewEffect->bExecuteOnApplication)
	{
		NewEffect->ExecuteEffect();
	}

	SetComponentTickEnabled(true);

	Result.bSuccess = true;
	Result.EffectHandle = NewEffect->GetEffectHandle();
	Result.ResultType = EZEffectApplicationResult::NewlyApplied;
	OnEffectAdded.Broadcast(NewEffect);
	return Result;
}

bool UZEffectComponent::CanApplyEffect(TSubclassOf<UZEffect> EffectClass, const FZEffectContext& Context) const
{
	if (!EffectClass)
	{
		return false;
	}

	const UZEffect* ClassDefaults = GetDefault<UZEffect>(EffectClass);
	if (!ClassDefaults)
	{
		return false;
	}

	if (ActiveEffectTags.HasAny(ClassDefaults->BlockedByTags))
	{
		return false;
	}

	return true;
}

UZEffect* UZEffectComponent::FindStackableEffect(TSubclassOf<UZEffect> EffectClass, const FZEffectContext& Context) const
{
	const UZEffect* ClassDefaults = GetDefault<UZEffect>(EffectClass);
	if (!ClassDefaults || ClassDefaults->StackPolicy == EZEffectStackPolicy::Independent)
	{
		return nullptr;
	}

	for (UZEffect* Effect : ActiveEffects)
	{
		if (Effect && Effect->bIsActive && Effect->GetClass() == EffectClass
			&& Effect->SourceActor == Context.SourceActor)
		{
			return Effect;
		}
	}

	return nullptr;
}

bool UZEffectComponent::RemoveEffect(FGuid EffectHandle, EZEffectRemovalReason Reason)
{
	UZEffect* Effect = FindEffect(EffectHandle);
	if (!Effect || !Effect->bIsActive)
	{
		return false;
	}

	Effect->bIsActive = false;
	Effect->OnRemoved(Reason);
	ReturnEffectTags(Effect);

	if (bIsUpdatingEffects)
	{
		PendingRemovals.Add(EffectHandle);
	}
	else
	{
		ActiveEffects.RemoveSingleSwap(Effect);
	}

	OnEffectRemoved.Broadcast(Effect, Reason);
	return true;
}

int32 UZEffectComponent::RemoveEffectsByTag(FGameplayTagContainer EffectTags, EZEffectRemovalReason Reason)
{
	int32 RemovedCount = 0;

	TArray<FGuid> HandlesToRemove;
	for (const UZEffect* Effect : ActiveEffects)
	{
		if (Effect && Effect->bIsActive && EffectTags.HasTag(Effect->GetEffectTag()))
		{
			HandlesToRemove.Add(Effect->GetEffectHandle());
		}
	}

	for (const FGuid& Handle : HandlesToRemove)
	{
		if (RemoveEffect(Handle, Reason))
		{
			++RemovedCount;
		}
	}

	return RemovedCount;
}

int32 UZEffectComponent::RemoveEffectsOnDeath()
{
	int32 RemovedCount = 0;

	TArray<FGuid> HandlesToRemove;
	for (const UZEffect* Effect : ActiveEffects)
	{
		if (Effect && Effect->bIsActive && Effect->bRemoveOnDeath)
		{
			HandlesToRemove.Add(Effect->GetEffectHandle());
		}
	}

	for (const FGuid& Handle : HandlesToRemove)
	{
		if (RemoveEffect(Handle, EZEffectRemovalReason::Death))
		{
			++RemovedCount;
		}
	}

	return RemovedCount;
}

void UZEffectComponent::ClearAllEffects()
{
	for (UZEffect* Effect : ActiveEffects)
	{
		if (Effect && Effect->bIsActive)
		{
			Effect->bIsActive = false;
			Effect->OnRemoved(EZEffectRemovalReason::Removed);
			ReturnEffectTags(Effect);
		}
	}

	ActiveEffects.Reset();
	PendingApplications.Reset();
	PendingRemovals.Reset();
}

UZEffect* UZEffectComponent::FindEffect(FGuid EffectHandle) const
{
	for (UZEffect* Effect : ActiveEffects)
	{
		if (Effect && Effect->GetEffectHandle() == EffectHandle)
		{
			return Effect;
		}
	}

	return nullptr;
}

bool UZEffectComponent::HasEffect(FGameplayTag EffectTag) const
{
	for (const UZEffect* Effect : ActiveEffects)
	{
		if (Effect && Effect->bIsActive && Effect->GetEffectTag() == EffectTag)
		{
			return true;
		}
	}

	return false;
}

bool UZEffectComponent::HasEffectTag(FGameplayTag Tag) const
{
	return ActiveEffectTags.HasTag(Tag);
}

void UZEffectComponent::GrantEffectTags(UZEffect* Effect)
{
	if (!Effect)
	{
		return;
	}

	TArray<FGameplayTag> Tags;
	Effect->GrantedTags.GetGameplayTagArray(Tags);

	for (const FGameplayTag& Tag : Tags)
	{
		int32& Count = EffectTagCounts.FindOrAdd(Tag);
		++Count;

		if (Count == 1)
		{
			ActiveEffectTags.AddTag(Tag);
		}
	}
}

void UZEffectComponent::ReturnEffectTags(UZEffect* Effect)
{
	if (!Effect)
	{
		return;
	}

	TArray<FGameplayTag> Tags;
	Effect->GrantedTags.GetGameplayTagArray(Tags);

	for (const FGameplayTag& Tag : Tags)
	{
		int32* Count = EffectTagCounts.Find(Tag);
		if (!Count)
		{
			continue;
		}

		--(*Count);

		if (*Count <= 0)
		{
			EffectTagCounts.Remove(Tag);
			ActiveEffectTags.RemoveTag(Tag);
		}
	}
}

void UZEffectComponent::UpdateEffects(double ServerNow)
{
	bIsUpdatingEffects = true;

	for (UZEffect* Effect : ActiveEffects)
	{
		if (!Effect || !Effect->bIsActive)
		{
			continue;
		}

		if (Effect->Period > 0.f && ServerNow >= Effect->NextExecutionServerTime)
		{
			Effect->ExecuteEffect();

			// ExecuteEffect 도중 대상이 사망하는 등으로 제거가 요청될 수 있으므로
			// 다음 주기 시각을 갱신하기 전에 다시 확인한다.
			if (!Effect->bIsActive)
			{
				continue;
			}

			Effect->NextExecutionServerTime = ServerNow + Effect->Period;
		}

		if (Effect->DurationPolicy == EZEffectDurationPolicy::HasDuration && ServerNow >= Effect->EndServerTime)
		{
			RemoveEffect(Effect->GetEffectHandle(), EZEffectRemovalReason::Expired);
		}
	}

	bIsUpdatingEffects = false;
	ProcessPendingChanges();

	if (ActiveEffects.Num() == 0)
	{
		SetComponentTickEnabled(false);
	}
}

void UZEffectComponent::ProcessPendingChanges()
{
	if (PendingRemovals.Num() > 0)
	{
		ActiveEffects.RemoveAll([this](const UZEffect* Effect)
		{
			return !Effect || PendingRemovals.Contains(Effect->GetEffectHandle());
		});
		PendingRemovals.Reset();
	}

	if (PendingApplications.Num() > 0)
	{
		ActiveEffects.Append(PendingApplications);
		PendingApplications.Reset();
	}
}
