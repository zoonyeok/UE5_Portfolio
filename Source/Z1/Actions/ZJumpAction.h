// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actions/ZAction.h"
#include "ZJumpAction.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API UZJumpAction : public UZAction
{
	GENERATED_BODY()

public:

	UZJumpAction();

	virtual void StartAction(AActor* Instigator) override;

	virtual void StopAction(AActor* Instigator) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> JumpMontage;

	void OnJumpMontageEnded(UAnimMontage* Montage, bool bInterrupted);

};
