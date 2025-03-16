// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/NoExportTypes.h"
#include "InputActionValue.h"
#include "ZAction.generated.h"

class UWorld;
class UZActionComponent;
class UZAnimInstance;
class ACharacter;

UCLASS(Blueprintable)
class Z1_API UZAction : public UObject
{
	GENERATED_BODY()
public:
	// Start Immediately When added to an action component
	UPROPERTY(EditDefaultsOnly, Category = Action)
	bool bAutoStart;

	UPROPERTY(EditDefaultsOnly, Category = Action)
	bool bIsComboAction = false;

	UPROPERTY(EditDefaultsOnly, Category = Action)
	FName ActionName;

	/* Action nickname to start/stop without a reference to the object */
	UPROPERTY(EditDefaultsOnly, Category = "Action")
	FGameplayTag ActivationTag;

	UFUNCTION(BlueprintCallable, Category = Action)
	virtual void StartAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = Action)
	virtual void StopAction(AActor* Instigator);

	UFUNCTION(BlueprintCallable, Category = Action)
	virtual void UpdateAction(AActor* Instigator, float DeltaTime);
	
	UFUNCTION(BlueprintCallable, Category = Action)
	virtual bool CanStartAction(AActor* Instigator);

	FORCEINLINE FGameplayTagContainer& GetGrantsTags() { return GrantsTags; }
	FORCEINLINE FGameplayTagContainer& GetBlockedTags() { return BlockedTags; }

	UFUNCTION(BlueprintCallable, Category = Action)
	bool IsRunning() const;

	UFUNCTION(BlueprintCallable, Category = Action)
	bool IsBeUpdated() const;
	
	virtual UWorld* GetWorld() const override;

	void Initialize(UZActionComponent* NewStateComponent);

	UFUNCTION(BlueprintCallable, Category = "Components")
	UZActionComponent* GetOwningComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Components")
	UZAnimInstance* GetOwningAnimInstance() const;

	UFUNCTION(BlueprintCallable, Category = "Components")
	ACharacter* GetCharacter() const;

protected:

	UPROPERTY(EditDefaultsOnly, Category = "Components")
	TObjectPtr<UZActionComponent> StateComponent;

	/* Action Can only start if OwningActor has none of these tags applied */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer BlockedTags;

	/* Tags added to owning actor when activated, removed when action stops */
	UPROPERTY(EditDefaultsOnly, Category = Tags)
	FGameplayTagContainer GrantsTags;

	bool bIsRunning = false;
	bool bIsBeUpdate = false;
};
