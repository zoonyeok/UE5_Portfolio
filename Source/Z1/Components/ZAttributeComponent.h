// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZGameTypes.h"
#include "ZAttributeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnAttributeChanged, AActor*, InstigatorActor, UZAttributeComponent*, OwningComp, float, NewValue, float, Delta);

class AZPlayerState;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZAttributeComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	FCharacterStats GetFCharacterStats();

	void InitializeCharacterStats(FCharacterStats PlayerStats);

	bool ChangeCurrentHP(UObject* InstigatorActor, float DeltaHP);

	bool ChangeCurrentMana(UObject* InstigatorActor, float DeltaMana);

	bool ChangeCurrentBetaEnergy(UObject* InstigatorActor, float DeltaBetaEnergy);

	UFUNCTION(BlueprintCallable)
	float GetCurrentHpPercent() const;

	FORCEINLINE bool IsAlive() const { return CurrentHP > 0; }
	FORCEINLINE float GetCurrentHP() const { return CurrentHP; }
	FORCEINLINE float GetMaxHP() const { return CharacterStats.MaxHP; }
	FORCEINLINE float GetCurrentMP() const { return CurrentMana; }
	FORCEINLINE float GetMaxMana() const { return CharacterStats.MaxMana; }

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttributeChanged OnHpChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnAttributeChanged OnManaChanged;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentHP;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentMana;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentStamina;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentBetaEnergy;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float CurrentShield;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float AttackPower;

	UPROPERTY(EditDefaultsOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float ShieldAttackPower;
    
    UPROPERTY()
    FCharacterStats CharacterStats;
};
