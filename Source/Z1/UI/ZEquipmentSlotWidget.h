// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ZItemDropWidgetInterface.h"
#include "ZGameTypes.h"
#include "ZEquipmentSlotWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UZEquipmentManagerComponent;
class UZInventoryItemWidget;
class UZInventoryItem;
class UZSpatialInventoryComponent;
/**
 * 
 */
UCLASS()
class Z1_API UZEquipmentSlotWidget : public UUserWidget, public IZItemDropWidgetInterface
{
	GENERATED_BODY()

public:
	void InitializeEquipWidget(const TObjectPtr<UZEquipmentManagerComponent>& InEquipmentManagerComponent, const TObjectPtr<UZInventoryItem>& InventoryItem);
	virtual bool HandleDropItem(class UZInventoryItemWidget* DroppedWidget, const FVector2D& MousePosition) override;
	FORCEINLINE TObjectPtr<UZEquipmentManagerComponent> GetEquipComponent() const { return EquipmentManagerComponent; }
	FORCEINLINE EItemSlotType GetItemSlotType() const { return ItemSlotType; }
	FORCEINLINE TObjectPtr<UZInventoryItem> GetItem() const { return Item; }
	
protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> SlotBorder;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> SlotCanvasPanel;

	UPROPERTY()
	TObjectPtr<UZEquipmentManagerComponent> EquipmentManagerComponent;

	UPROPERTY()
	TObjectPtr<UZSpatialInventoryComponent> SpacialnventoryComponent;

	UPROPERTY()
	EItemSlotType ItemSlotType;

	UPROPERTY()
	TObjectPtr<UZInventoryItem> Item;

	UPROPERTY()
	float TileSize;

protected:
	void RefreshGrid();
};
