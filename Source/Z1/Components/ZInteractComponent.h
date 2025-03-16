// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZGameTypes.h"
#include "ZInteractComponent.generated.h"

class UZInteractWidget;
class AZWorldItem;
struct FInteractionResult;
class UZWorldItemWidget;

USTRUCT(BlueprintType)
struct FInteractionData
{
	GENERATED_BODY()

	FInteractionData()
	{
		CurrentItemComponent = nullptr;
		CurrentWorldItem = nullptr;
		LastInteractionCheckTime = 0.f;
		bInteractHeld = false;
	}

	UPROPERTY()
	class UWorldItemWidgetComponent* CurrentItemComponent;

	UPROPERTY()
	AZWorldItem* CurrentWorldItem;

	UPROPERTY()
	float LastInteractionCheckTime;

	UPROPERTY()
	uint8 bInteractHeld : 1;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class Z1_API UZInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction")
	FInteractionData InteractionData;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float InteractionCheckFrequency;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float TraceDistance;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	float TraceRadius;

	UPROPERTY(EditDefaultsOnly, Category = Trace)
	TEnumAsByte<ECollisionChannel> CollisionChannel;

	void PerformInteractionCheck();
	void CouldntFindInteractable();
	void FoundNewInteractable(UWorldItemWidgetComponent* WorldItemWidgetComponent, AZWorldItem* WorldItem);

	FORCEINLINE class UWorldItemWidgetComponent* GetCurrentWorldItemWidgetComponent() const { return InteractionData.CurrentItemComponent; }

	FORCEINLINE class AZWorldItem* GetCurrentWorldItem() const { return InteractionData.CurrentWorldItem; }

	FTimerHandle TimerHandle_Interact;

public:

//PREV
	void PrimaryInteract();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FInteractionResult* FindInteractableActor();

	void DrawDebugInfo(const FVector& Start, const FVector& End, const TArray<FHitResult>& Hits, bool bBlockingHit) const;

//NEW
	// Tell the server we are interact with something
	void BeginInteract();
	void EndInteract();
	void Interact();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerEndInteract();

	//True if we're interacting with an item that has an interaction time (for example a lamp takes 2 seconds to turn on)
	bool IsInteracting() const;

	// Get the time till we interact with the current interatable.
	float GetRemainingInteractionTime() const;

private:

	bool IsActorOutOfRange(AZWorldItem* WorldItem);
	bool IsActorInSight(AZWorldItem* FocusedActor);
};