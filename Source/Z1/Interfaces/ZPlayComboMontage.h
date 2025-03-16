// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "ZGameplayTags.h"
#include "ZPlayComboMontage.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UZPlayComboMontage : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class Z1_API IZPlayComboMontage
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void PlayComboMontage(FGameplayTag ComboTag);
};
