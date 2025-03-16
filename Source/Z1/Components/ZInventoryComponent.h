// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZInventoryComponent.generated.h"

class AZBaseWeapon;
class AZMeleeWeaponBase;
class UZInventoryItem;
struct FGameplayTag;
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZInventoryComponent();
	
	//TODO : return EInventoryActionResult으로 변경;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UZInventoryItem* Item);

	//TODO : return EInventoryActionResult으로 변경;
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(UZInventoryItem* Item);

	//TODO : return EInventoryActionResult으로 변경;
	UFUNCTION(BlueprintCallable, Category = "Inventory|Slots")
	bool AddItemToWeaponSlot(UZInventoryItem* NewItem, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Filter")
	TArray<UZInventoryItem*> GetFilteredItems(const FGameplayTag& FilterCategory);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", Meta = (AllowPrivateAccess = true))
	TMap<FName, TObjectPtr<UZInventoryItem>> InventoryItems;

	// 무기 슬롯 배열 (근접, 원거리 상관없이 2개까지 착용 가능)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory|Slots", Meta = (AllowPrivateAccess = true))
	TArray<TObjectPtr<UZInventoryItem>> WeaponSlots;
};
