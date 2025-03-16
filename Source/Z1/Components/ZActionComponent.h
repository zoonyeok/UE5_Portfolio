// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "ZActionComponent.generated.h"

class UZAction;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnActionStateChanged, UZActionComponent*, OwningComp, UZAction*, Action);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZActionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZActionComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = Actions)
	TArray<TSubclassOf<UZAction>> DefaultActions;

public:
	UFUNCTION(BlueprintCallable, Category = Actions)
	void AddAction(AActor* Instigator, TSubclassOf<UZAction> ActionClass);

	UFUNCTION(BlueprintCallable, Category = Actions)
	bool RemoveAction(FName ActionName);

	UFUNCTION(BlueprintCallable, Category = Actions)
	bool StartActionByName(AActor* Instigator, FName ActionName);

	UFUNCTION(BlueprintCallable, Category = Actions)
	bool StopActionByName(AActor* Instigator, FName ActionName);

	//TODO : ActionByTag
	UFUNCTION(BlueprintCallable, Category = Actions)
	bool StartActionByTag(AActor* Instigator, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = Actions)
	bool StopActionByTag(AActor* Instigator, FGameplayTag ActionTag);

	UFUNCTION(BlueprintCallable, Category = Actions)
	UZAction* FindActionByName(FName ActionName);

	UFUNCTION(BlueprintCallable, Category = Actions)
	UZAction* FindActionByTag(FGameplayTag TagName);

	/*UFUNCTION(BlueprintCallable, Category = Actions)
	UTLAAction* FindActionByTagName(FName TagName);*/

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer ActiveGameplayTags;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ActionsData, meta = (AllowPrivateAccess = "true"))
	TArray<UZAction*> Actions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ActionsData, meta = (AllowPrivateAccess = "true"))
	TMap<FName, UZAction*> ActionsMap;

	UPROPERTY()
	TMap<FGameplayTag, UZAction*> ActionsTagMap;
};
