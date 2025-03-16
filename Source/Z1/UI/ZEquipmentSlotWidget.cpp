// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZEquipmentSlotWidget.h"
#include "UI/ZInventoryItemWidget.h"
#include "Items/ZInventoryItem.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/ZSpatialInventoryComponent.h"
#include <Player/ZPlayerController.h>
#include "Z1Character.h"

void UZEquipmentSlotWidget::InitializeEquipWidget(const TObjectPtr<UZEquipmentManagerComponent>& InEquipmentManagerComponent,
                                             const TObjectPtr<UZInventoryItem>& InventoryItem)
{
	if (InventoryItem)
	{
		if (FItemStaticData* ItemData = InventoryItem.Get()->GetStaticData())
		{
			ItemSlotType = ItemData->ItemSlotType;
		}
	}

	//TODO
	if (APlayerController* Controller = Cast<AZPlayerController>(GetOwningPlayer()))
	{
		if (AZ1Character* Character = Cast<AZ1Character>(Controller->GetCharacter()))
		{
			SpacialnventoryComponent = Character->GetComponentByClass<UZSpatialInventoryComponent>();

			int a = 0;
		}
	}
	
	EquipmentManagerComponent = InEquipmentManagerComponent;
}

bool UZEquipmentSlotWidget::HandleDropItem(UZInventoryItemWidget* DroppedWidget, const FVector2D& InMousePosition)
{
	FGeometry EquipmentGeometry = GetCachedGeometry();
	if (!EquipmentGeometry.IsUnderLocation(InMousePosition))
	{
		UE_LOG(LogTemp, Warning, TEXT("Mouse is not over EquipmentGridWidget!"));
		return false;
	}

	if (TObjectPtr<UZInventoryItem> InventoryItem = DroppedWidget->GetInventoryItem())
	{
		if (FItemStaticData* ItemData = InventoryItem->GetStaticData())
		{
			EItemSlotType WidgetItemSlotType = ItemData->ItemSlotType;

			if (ItemSlotType != WidgetItemSlotType)
			{
				return false;
			}

			if (EquipmentManagerComponent->EquipItem(WidgetItemSlotType, InventoryItem))
			{
				if (UCanvasPanelSlot* SlotPanel = Cast<UCanvasPanelSlot>(SlotCanvasPanel.Get()->AddChild(DroppedWidget)))
				{
					SlotPanel->SetAutoSize(true);
					SlotPanel->SetPosition(FVector2D::ZeroVector);
				}

				Item = InventoryItem;
				DroppedWidget->SetParentWidget(this);
				SpacialnventoryComponent->RemoveItemByPointer(InventoryItem);
				return true;
			}
		}
	}

	return false;
}

void UZEquipmentSlotWidget::RefreshGrid()
{
}
