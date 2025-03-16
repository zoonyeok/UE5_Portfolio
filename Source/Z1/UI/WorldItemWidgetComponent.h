// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "WorldItemWidgetComponent.generated.h"

class AZ1Character;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, AZ1Character*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, AZ1Character*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, AZ1Character*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, AZ1Character*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, AZ1Character*, Character);
/**
 * 
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class Z1_API UWorldItemWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
public:
	UWorldItemWidgetComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float InteractionDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableNameText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	FText InteractableActionText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	uint8 bAllowMultipleInteractors : 1;

	//Call this to change the name of the interactable. Will also refresh the interaction widget.
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void SetInteractableActionText(const FText& NewActionText);

	// Delegates
	// [local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	//[local + server] Called when the player has interacted with the item for the required amount of time
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnInteract OnInteract;
protected:

	//Called when game started
	virtual void Deactivate() override;

	bool CanInteract(AZ1Character* Character) const;

	//On the server, this will hold all interactors. On the local player, this will just hold the local player (provided they are an interactor)
	UPROPERTY()
	TArray<AZ1Character*> Interactors;

public:

	//Update only when UI updated, not every tick
	void RefreshWidget();

	//Called on the client when the players interaction check trace begin/ends hitting this item.
	void BeginFocus(AZ1Character* Character);
	void EndFocus(AZ1Character* Character);

	//Called on the client when the players begins/end interaction with the item.
	void BeginInteract(AZ1Character* Character);
	void EndInteract(AZ1Character* Character);

	void Interact(AZ1Character* Character);

	//0-1 denoting how far through the interact we are.
	UFUNCTION(BlueprintPure, Category = "Interaction")
	float GetInteractPercentage();
};
