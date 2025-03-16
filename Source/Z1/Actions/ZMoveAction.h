// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZAction.h"
#include "ZMoveAction.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API UZMoveAction : public UZAction
{
	GENERATED_BODY()
public:

	UZMoveAction();

	virtual void StartAction(AActor* Instigator) override;

	virtual void StopAction(AActor* Instigator) override;
	
	void Move(const FInputActionValue& Value);
	
};
