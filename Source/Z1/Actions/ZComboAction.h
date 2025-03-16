// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actions/ZAction.h"
#include "ZComboAction.generated.h"

class AZMeleeWeaponBase;
class UZComboActionData;

DECLARE_LOG_CATEGORY_EXTERN(LogComboAction, Log, All);

/**
 * 
 */
UCLASS()
class Z1_API UZComboAction : public UZAction
{
	GENERATED_BODY()

public:
	UZComboAction();

	virtual void StartAction(AActor* Instigator) override;

	virtual void StopAction(AActor* Instigator) override;

	void SetDefaultCombos();
	void SetNextComboSegment(FName LightCombo, FName HeavyCombo);
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combo, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> ComboMontage;

	UPROPERTY()
	FName LightComboAttack;
	UPROPERTY()
	FName HeavyComboAttack;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Combo, meta = (AllowPrivateAccess = true))
	FName DefaultLightComboAttack;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Combo, meta = (AllowPrivateAccess = true))
	FName DefaultHeavyComboAttack;
};


