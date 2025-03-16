// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ZItemDropWidgetInterface.h"
#include "ZInventoryGridWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UZSpatialInventoryComponent;
class UZInventoryItemWidget;
class UZInventoryItem;
class UZSpacialInventoryWidget;
/**
 * 
 */

DECLARE_LOG_CATEGORY_EXTERN(LogZGridWidget, Log, All);

USTRUCT(BlueprintType)
struct FLine
{
	GENERATED_BODY()

	// 기본 생성자
	FLine() : Start(FVector2D::ZeroVector), End(FVector2D::ZeroVector) {}

	FLine(double StartX, double StartY, double EndX, double EndY)
	{
		Start = FVector2D(StartX, StartY);
		End = FVector2D(EndX, EndY);
	}

	FVector2D Start;
	FVector2D End;
};


UCLASS()
class Z1_API UZInventoryGridWidget : public UUserWidget, public IZItemDropWidgetInterface
{
	GENERATED_BODY()
	
public:
	
	void InitializeWidget(const TObjectPtr<UZSpacialInventoryWidget>& InInventoryWidget);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	
	UFUNCTION()
	virtual bool HandleDropItem(UZInventoryItemWidget* DroppedWidget, const FVector2D& MousePosition) override;

	void SetTileColorInGrid(const FVector2D& StartLocation, const FVector2D& ItemSize, const FColor& Color);

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> GridCanvasPanel;

	FORCEINLINE float GetTileSize() const { return TileSize; }
	FORCEINLINE TObjectPtr<UZSpatialInventoryComponent> GetInventoryComponent() const { return InventoryComponent; }
	FORCEINLINE TObjectPtr<UZSpacialInventoryWidget> GetSpacialInventoryWidget() const { return SpacialInventoryWidget; }

	UFUNCTION()
	FIntPoint CalculateDropPosition(const FVector2D& InMousePosition, const FVector2D& ItemGridSize, bool bCorrectionApplied = false);
protected:

	UFUNCTION(BlueprintCallable)
	void CreateLineSegments();

	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	//Rendering Icon image on top of grid
	UFUNCTION(BlueprintCallable)
	void RefreshGrid();

	bool IsItemDropHighlightOutOfBounds(const FIntPoint& ItemDropPosition, const FVector2D& ItemSize);

	void UpdateInventoryItemWidgetMap(UZInventoryItemWidget* ItemWidget, const FIntPoint& OldPosition, const FIntPoint& NewPosition);


protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> GridBorder;

	UPROPERTY()
	float TileSize;

	UPROPERTY()
	TObjectPtr<UZSpatialInventoryComponent> InventoryComponent;

	UPROPERTY()
	TObjectPtr<UZSpacialInventoryWidget> SpacialInventoryWidget;

	UPROPERTY()
	TArray<FLine> GridLines;

	UPROPERTY()
	FIntPoint CurrentItemPosition;

	UPROPERTY()
	FIntPoint PrevItemDropPosition;
	bool bIsHighlighted = false;

	UPROPERTY()
	FIntPoint HighlightedDropTile;

	UPROPERTY()
	FVector2D HighlightedTileLocation = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D HighlightedTileSize = FVector2D::ZeroVector;

	UPROPERTY()
	FVector2D GridSize;

	UPROPERTY()
	FColor HighlightedTileColor = FColor(0.004183, 0.050347, 0.006972, 0.8);
//
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UZInventoryItemWidget> InventoryItemWidgetClass;

	UPROPERTY()
	TMap<FIntPoint, UZInventoryItemWidget*> InventoryItemWidgetMap;

private:
	void HandleOutOfBoundsDrop(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem);
	void HandleSwap(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem, 
		const FVector2D& InMousePosition, const FIntPoint& ItemDropPos);
	void HandleAddItem(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem,
		const FIntPoint& ItemDropPos);


};

