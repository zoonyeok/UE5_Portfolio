// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZEquipmentGridWidget.h"
#include "UI/ZEquipmentSlotWidget.h"
#include "UI/ZInventoryItemWidget.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "Items/ZInventoryItem.h"


void UZEquipmentGridWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    EquipmentSlotMap.Add(EItemSlotType::EIS_Helmet, HelmetSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Armor, ArmorSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Belt, BeltSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Gloves, GloveSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Boots, BootSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Weapon_1, WeaponSlot_1);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Weapon_2, WeaponSlot_2);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Ring_1, RingSlot_1);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Ring_2, RingSlot_2);
    EquipmentSlotMap.Add(EItemSlotType::EIS_Necklace, NecklaceSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_HealthPotion, HealthPotionSlot);
    EquipmentSlotMap.Add(EItemSlotType::EIS_ManaPotion, ManaPotionSlot);
}

void UZEquipmentGridWidget::InitializeEquipWidget(const TObjectPtr<UZEquipmentManagerComponent>& InEquipmentComponent)
{
    EquipmentComponent = InEquipmentComponent;
}

bool UZEquipmentGridWidget::HandleDropItem(UZInventoryItemWidget* DroppedWidget, const FVector2D& MousePosition)
{
    FGeometry EquipmentGeometry = GetCachedGeometry();
    if (!EquipmentGeometry.IsUnderLocation(MousePosition))
    {
        UE_LOG(LogTemp, Warning, TEXT("Mouse is not over EquipmentGridWidget!"));
        return false;
    }

    if (!DroppedWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("DroppedWidget is null in HandleDropItem."));
        return false;
    }

    if (TObjectPtr<UZInventoryItem> InventoryItem = DroppedWidget->GetInventoryItem())
    {
        if (FItemStaticData* ItemData = InventoryItem->GetStaticData())
        {
            EItemSlotType ItemSlotType = ItemData->ItemSlotType;

            TObjectPtr<UZEquipmentSlotWidget>* FoundSlotWidget = EquipmentSlotMap.Find(ItemSlotType);
            if (FoundSlotWidget && *FoundSlotWidget)
            {
                // TODO
                FGeometry SlotGeometry = FoundSlotWidget->Get()->GetCachedGeometry();
                if (!SlotGeometry.IsUnderLocation(MousePosition))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Mouse is not over SlotGeometry!"));
                    return false;
                }

                //TODO
                FoundSlotWidget->Get()->SetOwningPlayer(GetOwningPlayer());
                FoundSlotWidget->Get()->InitializeEquipWidget(EquipmentComponent, InventoryItem);
                if (!FoundSlotWidget->Get()->HandleDropItem(DroppedWidget, MousePosition))
                {
                    return false;
                }
                
                return true;
            }
        }
    }

    return false;
}
