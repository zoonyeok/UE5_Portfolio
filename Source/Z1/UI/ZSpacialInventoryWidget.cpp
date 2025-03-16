// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZSpacialInventoryWidget.h"
#include "UI/ZInventoryGridWidget.h"
#include "UI/ZEquipmentGridWidget.h"
#include "Items/ZInventoryItem.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "UI/ZInventoryItemWidget.h"
#include "GameFrameWork/ZGameStateBase.h"
#include "Components/ZEquipmentManagerComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogZSpacialInvenWidget, All, All)

void UZSpacialInventoryWidget::InitializeWidget(UZSpatialInventoryComponent* InInventoryComponent, UZEquipmentManagerComponent* InEquipmentComponent)
{
	InventoryComponent = InInventoryComponent;

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
	if (UZInventoryItemWidget* ItemWidget = Cast<UZInventoryItemWidget>(InOperation->Payload))
	{
		HandleDropItem(ItemWidget);
	}
	
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UZSpacialInventoryWidget::HandleDropItem(UZInventoryItemWidget* DroppedWidget)
{
	if (UZInventoryItemWidget* ItemWidget = Cast<UZInventoryItemWidget>(DroppedWidget))
	{
		if (TObjectPtr<UZInventoryItem> ItemToSpawn = ItemWidget->GetInventoryItem())
		{
			if (UWorld* World = GetWorld())
			{
				if (AZGameStateBase* GameState = Cast<AZGameStateBase>(World->GetGameState()))
				{
					bool bItemSpawned = GameState->SpawnWorldItem(ItemToSpawn, InventoryComponent->GetOwner()); //ItemToSpawn->SpawnWorldItem()
					if (bItemSpawned)
					{
						UE_LOG(LogZSpacialInvenWidget, Log, TEXT("ItemSpawn Success!"));
					}
					else
					{
						UE_LOG(LogZSpacialInvenWidget, Log, TEXT("ItemSpawn Failed!"));
					}
				}
			}
		}
	}
}
