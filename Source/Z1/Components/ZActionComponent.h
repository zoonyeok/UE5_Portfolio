// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	// Network Functions
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartActionByName(FName ActionName);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopActionByName(FName ActionName);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStartActionByTag(FGameplayTag ActionTag);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerStopActionByTag(FGameplayTag ActionTag);

	// Replication callbacks
	UFUNCTION()
	void OnRep_ActiveGameplayTags();

	UFUNCTION(BlueprintCallable, Category = Actions)
	UZAction* FindActionByName(FName ActionName);

	UFUNCTION(BlueprintCallable, Category = Actions)
	UZAction* FindActionByTag(FGameplayTag TagName);

	/*UFUNCTION(BlueprintCallable, Category = Actions)
	UTLAAction* FindActionByTagName(FName TagName);*/

	UPROPERTY(ReplicatedUsing = OnRep_ActiveGameplayTags, EditAnywhere, BlueprintReadWrite, Category = Tags)
	FGameplayTagContainer ActiveGameplayTags;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStarted;

	UPROPERTY(BlueprintAssignable)
	FOnActionStateChanged OnActionStopped;

protected:
	// Network validation helpers
	bool IsActionValidForNetworking(UZAction* Action) const;
	bool CanStartActionOnServer(AActor* Instigator, UZAction* Action) const;

	// Internal action management
	void StartActionInternal(AActor* Instigator, UZAction* Action);
	void StopActionInternal(AActor* Instigator, UZAction* Action);

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ActionsData, meta = (AllowPrivateAccess = "true"))
	TArray<UZAction*> Actions;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = ActionsData, meta = (AllowPrivateAccess = "true"))
	TMap<FName, UZAction*> ActionsMap;

	UPROPERTY()
	TMap<FGameplayTag, UZAction*> ActionsTagMap;

	// Network state tracking
	UPROPERTY()
	TArray<FName> ReplicatedRunningActions;

	// Previous state for replication callbacks
	FGameplayTagContainer PreviousActiveGameplayTags;
};
