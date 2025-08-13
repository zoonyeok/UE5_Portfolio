// Fill out your copyright notice in the Description page of Project Settings.

#include "ZCombatStateComponent.h"
#include "ZActionComponent.h"
#include "Actions/ZAction.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/Engine.h"
#include "TimerManager.h"

UZCombatStateComponent::UZCombatStateComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetIsReplicatedByDefault(true);

	CurrentState = ECombatState::Idle;
	bShowDebugInfo = false;

	InitializeStateData();
	InitializeTransitionRules();
}

void UZCombatStateComponent::BeginPlay()
{
	Super::BeginPlay();

	// Cache Action Component
	CachedActionComponent = GetActionComponent();

	// Bind to Action Component events if available
	if (CachedActionComponent)
	{
		CachedActionComponent->OnActionStarted.AddDynamic(this, &UZCombatStateComponent::NotifyActionStarted);
		CachedActionComponent->OnActionStopped.AddDynamic(this, &UZCombatStateComponent::NotifyActionStopped);
	}

	// Set initial state
	SetCombatStateInternal(ECombatState::Idle);
}

void UZCombatStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bShowDebugInfo)
	{
		DebugDrawCurrentState();
	}
}

void UZCombatStateComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZCombatStateComponent, CurrentState);
}

bool UZCombatStateComponent::TrySetCombatState(ECombatState NewState, bool bForceTransition)
{
	// Authority check for multiplayer
	if (GetOwner()->HasAuthority())
	{
		if (bForceTransition || CanTransitionToState(NewState))
		{
			SetCombatStateInternal(NewState);
			return true;
		}
		return false;
	}
	else
	{
		// Client: Send RPC to server
		ServerSetCombatState(NewState, bForceTransition);
		return true; // Assume success, server will validate
	}
}

bool UZCombatStateComponent::CanTransitionToState(ECombatState NewState) const
{
	// Already in the requested state
	if (CurrentState == NewState)
	{
		return true;
	}

	// Check if transition is allowed
	if (!IsValidTransition(CurrentState, NewState))
	{
		return false;
	}

	// Check current state data
	FCombatStateData CurrentStateData = GetCurrentStateData();
	if (!CurrentStateData.bCanBeInterrupted && NewState != ECombatState::Dead)
	{
		return false;
	}

	// Check if any actions are blocking the transition
	if (CachedActionComponent)
	{
		FGameplayTagContainer ActiveTags = CachedActionComponent->ActiveGameplayTags;
		FCombatStateData NewStateData = GetStateData(NewState);
		
		if (ActiveTags.HasAny(NewStateData.BlockedTags))
		{
			return false;
		}
	}

	return true;
}

bool UZCombatStateComponent::IsInAnyCombatState(const TArray<ECombatState>& States) const
{
	return States.Contains(CurrentState);
}

void UZCombatStateComponent::ResetToIdle()
{
	TrySetCombatState(ECombatState::Idle, true);
}

void UZCombatStateComponent::ServerSetCombatState_Implementation(ECombatState NewState, bool bForceTransition)
{
	TrySetCombatState(NewState, bForceTransition);
}

bool UZCombatStateComponent::ServerSetCombatState_Validate(ECombatState NewState, bool bForceTransition)
{
	// Basic validation - can be expanded
	return true;
}

void UZCombatStateComponent::OnRep_CurrentState(ECombatState OldState)
{
	OnCombatStateChanged.Broadcast(this, OldState, CurrentState);
	OnStateExited(OldState);
	OnStateEntered(CurrentState);
}

FCombatStateData UZCombatStateComponent::GetStateData(ECombatState State) const
{
	if (const FCombatStateData* StateData = StateDataMap.Find(State))
	{
		return *StateData;
	}
	return FCombatStateData(); // Return default if not found
}

FCombatStateData UZCombatStateComponent::GetCurrentStateData() const
{
	return GetStateData(CurrentState);
}

void UZCombatStateComponent::SetStateTimer(float Duration)
{
	if (Duration > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(StateTimerHandle, this, &UZCombatStateComponent::OnStateTimerExpired, Duration, false);
	}
}

void UZCombatStateComponent::ClearStateTimer()
{
	GetWorld()->GetTimerManager().ClearTimer(StateTimerHandle);
}

void UZCombatStateComponent::NotifyActionStarted(UZAction* Action)
{
	if (!Action) return;

	// Determine combat state based on action
	FGameplayTag ActionTag = Action->ActivationTag;
	
	// Example mapping - customize based on your action tags
	if (ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Action.Attack"))))
	{
		TrySetCombatState(ECombatState::Attacking);
	}
	else if (ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Action.Block"))))
	{
		TrySetCombatState(ECombatState::Blocking);
	}
	else if (ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Action.Dodge"))))
	{
		TrySetCombatState(ECombatState::Dodging);
	}
	else if (ActionTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Action.Parry"))))
	{
		TrySetCombatState(ECombatState::Parrying);
	}
}

void UZCombatStateComponent::NotifyActionStopped(UZAction* Action)
{
	if (!Action) return;

	// Return to idle if no other actions are running
	if (CachedActionComponent)
	{
		bool bHasActiveActions = false;
		for (UZAction* ActiveAction : CachedActionComponent->Actions)
		{
			if (ActiveAction && ActiveAction->IsRunning() && ActiveAction != Action)
			{
				bHasActiveActions = true;
				break;
			}
		}

		if (!bHasActiveActions)
		{
			TrySetCombatState(ECombatState::Idle);
		}
	}
}

void UZCombatStateComponent::DebugDrawCurrentState()
{
	if (GEngine && GetOwner())
	{
		FString StateString = FString::Printf(TEXT("Combat State: %s"), 
			*UEnum::GetValueAsString(CurrentState));
		
		GEngine->AddOnScreenDebugMessage(
			GetOwner()->GetUniqueID(),
			0.0f,
			FColor::Yellow,
			StateString
		);
	}
}

void UZCombatStateComponent::SetCombatStateInternal(ECombatState NewState)
{
	if (CurrentState == NewState) return;

	ECombatState OldState = CurrentState;
	CurrentState = NewState;

	// Handle state change
	OnStateExited(OldState);
	OnStateEntered(NewState);

	// Broadcast state change
	OnCombatStateChanged.Broadcast(this, OldState, NewState);

	// Update gameplay tags
	UpdateActionComponentTags();
}

void UZCombatStateComponent::OnStateEntered(ECombatState NewState)
{
	FCombatStateData StateData = GetStateData(NewState);

	// Apply movement speed modifier
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		if (UCharacterMovementComponent* MovementComp = Character->GetCharacterMovement())
		{
			float BaseSpeed = MovementComp->MaxWalkSpeed;
			MovementComp->MaxWalkSpeed = BaseSpeed * StateData.MovementSpeedModifier;
		}
	}

	// Grant gameplay tags
	if (CachedActionComponent && StateData.GrantedTags.Num() > 0)
	{
		for (const FGameplayTag& Tag : StateData.GrantedTags)
		{
			CachedActionComponent->ActiveGameplayTags.AddTag(Tag);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Combat State Entered: %s"), *UEnum::GetValueAsString(NewState));
}

void UZCombatStateComponent::OnStateExited(ECombatState OldState)
{
	FCombatStateData StateData = GetStateData(OldState);

	// Remove granted gameplay tags
	if (CachedActionComponent && StateData.GrantedTags.Num() > 0)
	{
		for (const FGameplayTag& Tag : StateData.GrantedTags)
		{
			CachedActionComponent->ActiveGameplayTags.RemoveTag(Tag);
		}
	}

	// Clear any active timers
	ClearStateTimer();

	UE_LOG(LogTemp, Log, TEXT("Combat State Exited: %s"), *UEnum::GetValueAsString(OldState));
}

void UZCombatStateComponent::OnStateTimerExpired()
{
	// Default behavior: return to idle
	TrySetCombatState(ECombatState::Idle);
}

void UZCombatStateComponent::UpdateActionComponentTags()
{
	if (!CachedActionComponent) return;

	// This could be expanded to sync tags between combat state and action system
	// For now, it's handled in OnStateEntered/OnStateExited
}

UZActionComponent* UZCombatStateComponent::GetActionComponent() const
{
	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UZActionComponent>();
	}
	return nullptr;
}

void UZCombatStateComponent::InitializeStateData()
{
	// Initialize default state data
	FCombatStateData IdleData;
	IdleData.State = ECombatState::Idle;
	IdleData.bCanBeInterrupted = true;
	IdleData.MovementSpeedModifier = 1.0f;
	StateDataMap.Add(ECombatState::Idle, IdleData);

	FCombatStateData AttackingData;
	AttackingData.State = ECombatState::Attacking;
	AttackingData.bCanBeInterrupted = false;
	AttackingData.MovementSpeedModifier = 0.3f;
	AttackingData.BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action.Move")));
	AttackingData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Attacking")));
	StateDataMap.Add(ECombatState::Attacking, AttackingData);

	FCombatStateData BlockingData;
	BlockingData.State = ECombatState::Blocking;
	BlockingData.bCanBeInterrupted = true;
	BlockingData.MovementSpeedModifier = 0.5f;
	BlockingData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Blocking")));
	StateDataMap.Add(ECombatState::Blocking, BlockingData);

	FCombatStateData DodgingData;
	DodgingData.State = ECombatState::Dodging;
	DodgingData.bCanBeInterrupted = false;
	DodgingData.MovementSpeedModifier = 1.5f;
	DodgingData.BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action.Attack")));
	DodgingData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dodging")));
	StateDataMap.Add(ECombatState::Dodging, DodgingData);

	FCombatStateData StunnedData;
	StunnedData.State = ECombatState::Stunned;
	StunnedData.bCanBeInterrupted = false;
	StunnedData.MovementSpeedModifier = 0.0f;
	StunnedData.BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action")));
	StunnedData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Stunned")));
	StateDataMap.Add(ECombatState::Stunned, StunnedData);

	FCombatStateData ParryingData;
	ParryingData.State = ECombatState::Parrying;
	ParryingData.bCanBeInterrupted = false;
	ParryingData.MovementSpeedModifier = 0.1f;
	ParryingData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Parrying")));
	StateDataMap.Add(ECombatState::Parrying, ParryingData);

	FCombatStateData ExecutingData;
	ExecutingData.State = ECombatState::Executing;
	ExecutingData.bCanBeInterrupted = false;
	ExecutingData.MovementSpeedModifier = 0.0f;
	ExecutingData.BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action")));
	ExecutingData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Executing")));
	StateDataMap.Add(ECombatState::Executing, ExecutingData);

	FCombatStateData DeadData;
	DeadData.State = ECombatState::Dead;
	DeadData.bCanBeInterrupted = false;
	DeadData.MovementSpeedModifier = 0.0f;
	DeadData.BlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Action")));
	DeadData.GrantedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Dead")));
	StateDataMap.Add(ECombatState::Dead, DeadData);
}

void UZCombatStateComponent::InitializeTransitionRules()
{
	// Define allowed state transitions
	AllowedTransitions.Empty();

	// From Idle
	AllowedTransitions.Add(ECombatState::Idle, {
		ECombatState::Attacking,
		ECombatState::Blocking,
		ECombatState::Dodging,
		ECombatState::Parrying,
		ECombatState::Stunned,
		ECombatState::Dead
	});

	// From Attacking
	AllowedTransitions.Add(ECombatState::Attacking, {
		ECombatState::Idle,
		ECombatState::Attacking,  // Allow combo attacks
		ECombatState::Stunned,
		ECombatState::Dead
	});

	// From Blocking
	AllowedTransitions.Add(ECombatState::Blocking, {
		ECombatState::Idle,
		ECombatState::Attacking,
		ECombatState::Dodging,
		ECombatState::Parrying,
		ECombatState::Stunned,
		ECombatState::Dead
	});

	// From Dodging
	AllowedTransitions.Add(ECombatState::Dodging, {
		ECombatState::Idle,
		ECombatState::Attacking,
		ECombatState::Dead
	});

	// From Stunned
	AllowedTransitions.Add(ECombatState::Stunned, {
		ECombatState::Idle,
		ECombatState::Dead
	});

	// From Parrying
	AllowedTransitions.Add(ECombatState::Parrying, {
		ECombatState::Idle,
		ECombatState::Attacking,
		ECombatState::Executing,
		ECombatState::Dead
	});

	// From Executing
	AllowedTransitions.Add(ECombatState::Executing, {
		ECombatState::Idle,
		ECombatState::Dead
	});

	// From Dead (only revive possible)
	AllowedTransitions.Add(ECombatState::Dead, {
		ECombatState::Idle  // For respawn/revive
	});
}

bool UZCombatStateComponent::IsValidTransition(ECombatState From, ECombatState To) const
{
	if (const TArray<ECombatState>* AllowedStates = AllowedTransitions.Find(From))
	{
		return AllowedStates->Contains(To);
	}
	return false;
}