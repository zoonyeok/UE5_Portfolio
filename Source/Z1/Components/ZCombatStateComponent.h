// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
#include "ZGameTypes.h"
#include "ZCombatStateComponent.generated.h"

class UZActionComponent;

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Blocking	UMETA(DisplayName = "Blocking"),
	Dodging		UMETA(DisplayName = "Dodging"),
	Stunned		UMETA(DisplayName = "Stunned"),
	Parrying	UMETA(DisplayName = "Parrying"),
	Executing	UMETA(DisplayName = "Executing"),
	Dead		UMETA(DisplayName = "Dead")
};

USTRUCT(BlueprintType)
struct FCombatStateData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	ECombatState State;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	FGameplayTagContainer BlockedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	FGameplayTagContainer GrantedTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	bool bCanBeInterrupted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	float MovementSpeedModifier;

	FCombatStateData()
	{
		State = ECombatState::Idle;
		bCanBeInterrupted = true;
		MovementSpeedModifier = 1.0f;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCombatStateChanged, UZCombatStateComponent*, StateComponent, ECombatState, OldState, ECombatState, NewState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class Z1_API UZCombatStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZCombatStateComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:
	// State Management
	UFUNCTION(BlueprintCallable, Category = "Combat State")
	bool TrySetCombatState(ECombatState NewState, bool bForceTransition = false);

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	ECombatState GetCurrentCombatState() const { return CurrentState; }

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	bool CanTransitionToState(ECombatState NewState) const;

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	bool IsInCombatState(ECombatState State) const { return CurrentState == State; }

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	bool IsInAnyCombatState(const TArray<ECombatState>& States) const;

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	void ResetToIdle();

	// Network Functions
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerSetCombatState(ECombatState NewState, bool bForceTransition);

	UFUNCTION()
	void OnRep_CurrentState(ECombatState OldState);

	// State Data Access
	UFUNCTION(BlueprintCallable, Category = "Combat State")
	FCombatStateData GetStateData(ECombatState State) const;

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	FCombatStateData GetCurrentStateData() const;

	// Timer Management
	UFUNCTION(BlueprintCallable, Category = "Combat State")
	void SetStateTimer(float Duration);

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	void ClearStateTimer();

	// Integration with Action System
	UFUNCTION(BlueprintCallable, Category = "Combat State")
	void NotifyActionStarted(class UZAction* Action);

	UFUNCTION(BlueprintCallable, Category = "Combat State")
	void NotifyActionStopped(class UZAction* Action);

	// Events
	UPROPERTY(BlueprintAssignable)
	FOnCombatStateChanged OnCombatStateChanged;

	// Debug
	UFUNCTION(BlueprintCallable, Category = "Combat State", CallInEditor = true)
	void DebugDrawCurrentState();

protected:
	// State Management
	void SetCombatStateInternal(ECombatState NewState);
	void OnStateEntered(ECombatState NewState);
	void OnStateExited(ECombatState OldState);

	// Timer Callbacks
	UFUNCTION()
	void OnStateTimerExpired();

	// Integration
	void UpdateActionComponentTags();
	class UZActionComponent* GetActionComponent() const;

protected:
	// Current State
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState, BlueprintReadOnly, Category = "Combat State")
	ECombatState CurrentState;

	// State Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat State")
	TMap<ECombatState, FCombatStateData> StateDataMap;

	// Timer Handle for timed states
	FTimerHandle StateTimerHandle;

	// Cached Components
	UPROPERTY()
	TObjectPtr<UZActionComponent> CachedActionComponent;

	// State Transition Rules
	UPROPERTY(EditAnywhere, Category = "Combat State")
	TMap<ECombatState, TArray<ECombatState>> AllowedTransitions;

	// Debug
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bShowDebugInfo;

private:
	void InitializeStateData();
	void InitializeTransitionRules();
	bool IsValidTransition(ECombatState From, ECombatState To) const;
};