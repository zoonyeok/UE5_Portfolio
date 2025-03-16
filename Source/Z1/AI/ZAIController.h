// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ZAIController.generated.h"

class UBehaviorTree;
/**
 * Brain of AI
 */
UCLASS()
class Z1_API AZAIController : public AAIController
{
	GENERATED_BODY()
	
protected:

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	virtual void BeginPlay() override;
};
