// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actions/ZAction.h"
#include "ZRollAction.generated.h"

class UAnimMontage;
/**
 * 
 */
UCLASS()
class Z1_API UZRollAction : public UZAction
{
	GENERATED_BODY()
	
public:

	UZRollAction();

	virtual void StartAction(AActor* Instigator) override;

	virtual void StopAction(AActor* Instigator) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> RollMontage;

	void OnRollMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
