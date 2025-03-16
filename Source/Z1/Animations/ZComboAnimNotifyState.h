// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ZComboAnimNotifyState.generated.h"

class UZComboActionData;
/**
 * 
 */
UCLASS()
class Z1_API UZComboAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	UZComboAnimNotifyState();

	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Action)
	FGameplayTag ActionTag;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Action)
	FName NextLightComboName;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = Action)
	FName NextHeavyComboName;

	bool bCanNextCombo = false;
};
