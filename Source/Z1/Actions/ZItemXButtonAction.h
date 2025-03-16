// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZAction.h"
#include "ZItemXButtonAction.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API UZItemXButtonAction : public UZAction
{
	GENERATED_BODY()

public:
	UZItemXButtonAction();
	
	virtual void StartAction(AActor* Instigator) override;

	virtual void StopAction(AActor* Instigator) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Movement, meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> ItemUseMontage;

	void OnItemUseMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};
