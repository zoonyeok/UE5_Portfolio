// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZInventoryItemWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemRemoved, UZInventoryItem*, RemovedItem);

class UZInventoryItem;
class USizeBox;
class UImage;
class UBorder;
class UMaterialInstance;
class UZSpatialInventoryComponent;
class UZEquipmentGridWidget;
class UCanvasPanel;
class UZInventoryGridWidget;
/**
 *
 */

UCLASS()
class Z1_API UZInventoryItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UZInventoryItemWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// whenever Inventory Redrawn
	UFUNCTION(BlueprintCallable)
	void Refresh();

	void InitializeItemWidget(const TObjectPtr<UZInventoryGridWidget>& GridWidget, const TObjectPtr<UZInventoryItem>& Item);
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual bool NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	FORCEINLINE TObjectPtr<UZInventoryItem> GetInventoryItem() const { return InventoryItem; }
	FORCEINLINE TWeakObjectPtr<UUserWidget> GetParentWidget() const { return ParentWidget; }
	void SetParentWidget(TWeakObjectPtr<UUserWidget> NewParentWidget);
public:
	//bool
	UFUNCTION(BlueprintCallable)
	FSlateBrush GetIconImage() const;

	UPROPERTY(BlueprintAssignable, Category = "Delegates")
	FOnItemRemoved OnItemRemoved;
	
	void StartDragging(const FVector2D& MousePosition);
protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	void SetItemBackgroundColor();
	
protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ItemCanvasPanel;

	UPROPERTY(meta = (BindWidget))
	USizeBox* BackgroundSizeBox;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemImage;

	UPROPERTY(meta = (BindWidget))
	UBorder* BackgroundBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem", meta = (ExposeOnSpawn = true))
	TObjectPtr<UZInventoryGridWidget> GridWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem", meta = (ExposeOnSpawn = true))
	TObjectPtr<UZSpatialInventoryComponent> InventoryComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem", meta = (ExposeOnSpawn = true))
	TObjectPtr<UZEquipmentGridWidget> EquipmentGridWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem", meta = (ExposeOnSpawn = true))
	TObjectPtr<UZInventoryItem> InventoryItem;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem", meta = (ExposeOnSpawn = true))
	float TileSize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FVector2D ItemDisplaySize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FColor EquipableColor = FColor(0.004438, 0.004438, 0.050347, 0.8);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FColor HoveringItemColor = FColor(0.004183, 0.050347, 0.006972, 0.8);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FColor EquipdisableColor = FColor(0.258681, 0.005899, 0.008846, 0.8);

	//아이템 요구 능력치 만족에 따른 컬러 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	FColor CurrentColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	bool bIsDragging;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	bool bIsItemClicked;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryItem")
	TSubclassOf<UZInventoryGridWidget> GridWidgetClass;

	TWeakObjectPtr<UUserWidget> ParentWidget;

	bool bIsMousePressed = false;
	bool bToggleDragging = false;
	FVector2D InitialMousePosition;
	FVector2D DragOffset;
	float DragThreshold = 10.0f;
	double DragThresholdTime = 0.0;
	double PressedTime = 0.0;
	
	bool StopDragging(const FPointerEvent& InMouseEvent);
	void UpdateDragPosition(const FVector2D& MousePosition);
};

