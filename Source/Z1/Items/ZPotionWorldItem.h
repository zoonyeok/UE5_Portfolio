// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ZWorldItem.h"
#include "ZPotionWorldItem.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API AZPotionWorldItem : public AZWorldItem
{
	GENERATED_BODY()
	
public:
	AZPotionWorldItem();

	virtual EInventoryActionResult PickUp_Implementation(APawn* Player) override;

	virtual UZInventoryItem* ConvertToInventoryItem() override;
};
 