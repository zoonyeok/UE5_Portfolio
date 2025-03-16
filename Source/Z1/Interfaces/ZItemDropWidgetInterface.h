// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZItemDropWidgetInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UZItemDropWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class Z1_API IZItemDropWidgetInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool HandleDropItem(class UZInventoryItemWidget* DroppedWidget, const FVector2D& MousePosition) = 0;
};
