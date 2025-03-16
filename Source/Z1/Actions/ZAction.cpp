// Fill out your copyright notice in the Description page of Project Settings.
#include "Actions/ZAction.h"
#include "Components/ZActionComponent.h"
#include "Animations/ZAnimInstance.h"
#include "GameFramework/Character.h"

void UZAction::StartAction(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Running: %s"), *GetNameSafe(Instigator));
	bIsRunning = true;

	UZActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.AppendTags(GrantsTags);

	Comp->OnActionStarted.Broadcast(Comp, this);
}

void UZAction::StopAction(AActor* Instigator)
{
	UE_LOG(LogTemp, Log, TEXT("Running: %s"), *GetNameSafe(Instigator));
	bIsRunning = false;

	UZActionComponent* Comp = GetOwningComponent();
	Comp->ActiveGameplayTags.RemoveTags(GrantsTags);

	Comp->OnActionStopped.Broadcast(Comp, this);
}

void UZAction::UpdateAction(AActor* Instigator, float DeltaTime)
{
}

bool UZAction::CanStartAction(AActor* Instigator)
{
	UZActionComponent* Comp = GetOwningComponent();
	if (Comp->ActiveGameplayTags.HasAny(BlockedTags))
	{
		FString FailedMsg = FString::Printf(TEXT("Failed to run: %s"), *ActionName.ToString());
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, FailedMsg);
		return false;
	}

	// 콤보 액션인 경우 실행 가능
	if (bIsComboAction)
	{
		return true;
	}

	// 액션이 이미 실행 중이면 실행 불가
	if (bIsRunning)
	{
		return false;
	}

	return true;
}

bool UZAction::IsRunning() const
{
	return bIsRunning;
}

bool UZAction::IsBeUpdated() const
{
	return bIsBeUpdate;
}

UWorld* UZAction::GetWorld() const
{
	if (const AActor* Actor = Cast<AActor>(GetOuter()))
	{
		return Actor->GetWorld();
	}
	return nullptr;
}

void UZAction::Initialize(UZActionComponent* NewActionComponent)
{
	StateComponent = NewActionComponent;
}

UZActionComponent* UZAction::GetOwningComponent() const
{
	return StateComponent;
}

UZAnimInstance* UZAction::GetOwningAnimInstance() const
{
	if (const ACharacter* Character = GetCharacter())
	{
		if (UZAnimInstance* AnimInstance = Cast<UZAnimInstance>(Character->GetMesh()->GetAnimInstance()))
		{
			return AnimInstance;
		}
	}
	return nullptr;
}

ACharacter* UZAction::GetCharacter() const
{
	if (ACharacter* Character = Cast<ACharacter>(StateComponent->GetOwner()))
	{
		return Character;
	}
	return nullptr;
}
