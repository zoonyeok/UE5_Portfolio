// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actions/ZAction.h"
#include "ZDodgeAction.generated.h"

class UBlendSpace;
/**
 * 
 */
UCLASS()
class Z1_API UZDodgeAction : public UZAction
{
	GENERATED_BODY()

public:

	UZDodgeAction();

	virtual void StartAction(AActor* Instigator) override;

	virtual void StopAction(AActor* Instigator) override;

private:

};
