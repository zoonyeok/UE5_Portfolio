// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ZInventoryItem.h"
#include "Interfaces/ZUseable.h"
#include "Interfaces/ZDropable.h"
#include "ZPotionInventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API UZPotionInventoryItem : public UZInventoryItem, public IZUseable, public IZDropable
{
	GENERATED_BODY()
	
public:
	UZPotionInventoryItem();

	virtual void UseItem_Implementation(APawn* User) override;

	virtual EInventoryActionResult DropItem_Implementation(APawn* Dropper) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", Meta = (AllowPrivateAccess = "true"))
	int HealthAmount;
};
