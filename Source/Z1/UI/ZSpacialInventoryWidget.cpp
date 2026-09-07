// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZSpacialInventoryWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/ZInventoryGridWidget.h"
#include "UI/ZEquipmentGridWidget.h"
#include "Items/ZInventoryItem.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "UI/ZInventoryItemWidget.h"
#include "GameFrameWork/ZGameStateBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogZSpacialInvenWidget, All, All)

void UZSpacialInventoryWidget::InitializeWidget(UZSpatialInventoryComponent* InInventoryComponent, UZEquipmentManagerComponent* InEquipmentComponent)
{
	InventoryComponent = InInventoryComponent;
	DropTargets.Reset();

	if (IsValid(InventoryGridWidget))
	{
		InventoryGridWidget.Get()->InitializeWidget(this);
		InventoryGridWidget.Get()->SetOwningPlayer(GetOwningPlayer());
		DropTargets.Add(InventoryGridWidget);
	}

	if (IsValid(EquipmentGridWidget))
	{
		EquipmentGridWidget.Get()->InitializeEquipWidget(InEquipmentComponent);
		EquipmentGridWidget.Get()->SetOwningPlayer(GetOwningPlayer());
		DropTargets.Add(EquipmentGridWidget);
	}
}

bool UZSpacialInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (InOperation && HandleDropItem(Cast<UZInventoryItemWidget>(InOperation->Payload),
		InDragDropEvent.GetScreenSpacePosition()))
	{
		return true;
	}
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

bool UZSpacialInventoryWidget::IsOverInventoryPanels(const FVector2D& ScreenPosition) const
{
	return (IsValid(InventoryGridWidget) &&
		InventoryGridWidget->GetCachedGeometry().IsUnderLocation(ScreenPosition)) ||
		(IsValid(EquipmentGridWidget) &&
		EquipmentGridWidget->GetCachedGeometry().IsUnderLocation(ScreenPosition));
}

bool UZSpacialInventoryWidget::HandleDropItem(UZInventoryItemWidget* DroppedWidget, const FVector2D& ScreenPosition)
{
	// World drops are allowed only outside both panels, not the fullscreen root.
	if (IsOverInventoryPanels(ScreenPosition) ||
		!IsValid(DroppedWidget) || DraggedItemWidget != DroppedWidget ||
		!IsValid(InventoryComponent))
	{
		return false;
	}

	UZInventoryItem* ItemToSpawn = DroppedWidget->GetInventoryItem();
	UWorld* World = GetWorld();
	AZGameStateBase* GameState = World ? World->GetGameState<AZGameStateBase>() : nullptr;
	if (!IsValid(ItemToSpawn) || !IsValid(GameState) ||
		!GameState->SpawnWorldItem(ItemToSpawn, InventoryComponent->GetOwner()))
	{
		UE_LOG(LogZSpacialInvenWidget, Warning, TEXT("World drop failed; keeping the item on the cursor."));
		return false;
	}

	// Commit removal only after spawning succeeds. Also works for displaced swap items.
	DroppedWidget->CancelDragging();
	DraggedItemWidget = nullptr;
	InventoryComponent->RemoveItemByPointer(ItemToSpawn);
	return true;
}


// Cursor widgets are viewport siblings, so removing the inventory alone is insufficient.
void UZSpacialInventoryWidget::NativeDestruct()
{
	if (IsValid(DraggedItemWidget))
	{
		UZInventoryItemWidget* CursorWidget = DraggedItemWidget;
		UZInventoryItem* CursorItem = CursorWidget->GetInventoryItem();
		CursorWidget->CancelDragging();
		DraggedItemWidget = nullptr;
		if (IsValid(InventoryComponent) && IsValid(CursorItem) &&
			InventoryComponent->FindItemPosition(CursorItem).X < 0)
		{
			// A swap removes the displaced item from the grid. Keep it owned if full.
			if (!InventoryComponent->TryAddItem(CursorItem))
			{
				DraggedItemWidget = CursorWidget;
			}
		}
	}
	Super::NativeDestruct();
}

void UZSpacialInventoryWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (IsValid(DraggedItemWidget))
	{
		DraggedItemWidget->StartDragging(FSlateApplication::Get().GetCursorPos());
	}
	if (IsValid(InventoryComponent))
	{
		InventoryComponent->OnInventoryChanged.Broadcast();
	}
}
