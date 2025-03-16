// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ZInventoryItem.h"
#include "Interfaces/ZDropable.h"
#include "ZWeaponInventoryItem.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API UZWeaponInventoryItem : public UZInventoryItem, public IZDropable
{
	GENERATED_BODY()

public:
	UZWeaponInventoryItem();
	
	virtual EInventoryActionResult DropItem_Implementation(APawn* Dropper) override;

protected:

};
