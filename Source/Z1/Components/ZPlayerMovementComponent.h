// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "ZPlayerMovementComponent.generated.h"

class UZAnimInstance;

UENUM(BlueprintType)
enum ECustomMovementMode
{
	MOVE_Rolling UMETA(DisplayName = "Rolling Mode"),

};
/**
 * 
 */
UCLASS()
class Z1_API UZPlayerMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UZPlayerMovementComponent(const FObjectInitializer& ObjectInitializer);

	//virtual float GetMaxSpeed() const override;
	//virtual float GetMaxAcceleration() const override;

protected:

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	
private:

	UPROPERTY()
	TObjectPtr<UZAnimInstance> PlayerAnimInstance;
	TObjectPtr<UZAnimInstance> GetAnimInstance();
};
