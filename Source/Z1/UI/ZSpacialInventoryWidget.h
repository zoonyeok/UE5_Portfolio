// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZSpacialInventoryWidget.generated.h"

class UZInventoryItemWidget;
class UZSpatialInventoryComponent;
class UZInventoryGridWidget;
class UZEquipmentGridWidget;
class UZEquipmentManagerComponent;
/**
 * 
 */

UCLASS()
class Z1_API UZSpacialInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	void InitializeWidget(UZSpatialInventoryComponent* InInventoryComponent, UZEquipmentManagerComponent* InEquipmentComponent);
	void HandleDropItem(UZInventoryItemWidget* DroppedWidget);

	FORCEINLINE UZSpatialInventoryComponent* GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE float GetTileSize() const { return TileSize; }
	FORCEINLINE TArray<TObjectPtr<UUserWidget>> GetDropTargets() const { return DropTargets; }
	FORCEINLINE TObjectPtr<UZEquipmentGridWidget> GetEquipmentGridWidget() const { return EquipmentGridWidget; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZInventoryGridWidget> InventoryGridWidget;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentGridWidget> EquipmentGridWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ExposeOnSpawn = "true"))
	UZSpatialInventoryComponent* InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ExposeOnSpawn = "true"))
	float TileSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ExposeOnSpawn = "true"))
	TArray<TObjectPtr<UUserWidget>> DropTargets;

};
