// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ZInventoryItem.h"
#include "ZGameTypes.h"
#include "Z1Character.h"
#include "Items/ZWorldItem.h"
#include "Materials/MaterialInstanceDynamic.h"

void UZInventoryItem::Initialize(UDataTable* DataTable, FName RowName, int32 InitialQuantity)
{
    //LoadObject<UDataTable>(nullptr, TEXT("/Game/ItemDataTable.ItemDataTable"));
    ItemDataTable = DataTable;
    ItemRowName = RowName;

    ItemGridSize = GetStaticData()->ItemGridSize;

    InstanceData.Quantity = InitialQuantity;
    InstanceData.ItemPrice = GetStaticData()->ItemPrice;

    bIsStaticDataCached = false; // Reset cache

    IconMaterial = CreateItemIconDynamicMaterial();
    bIsRotate = false;
}

FItemStaticData* UZInventoryItem::GetStaticData() const
{
    if (!bIsStaticDataCached && ItemDataTable)
    {
        FItemStaticData* StaticData = ItemDataTable->FindRow<FItemStaticData>(ItemRowName, TEXT("GetStaticData"));
        if (StaticData)
        {
            CachedStaticData = *StaticData;
            bIsStaticDataCached = true;
        }
    }
    return bIsStaticDataCached ? &CachedStaticData : nullptr;
}

UMaterialInstanceDynamic* UZInventoryItem::GetIconMaterial(bool Rotate = false)
{
    if (Rotate)
    {
        float RotateValue = bIsRotate ? 0.0f : -0.25f;

        if (IconMaterial)
        {
            IconMaterial->SetScalarParameterValue(TEXT("RotationAngle"), RotateValue);
            bIsRotate = !bIsRotate;
        }
    }

    return IconMaterial;
}

bool UZInventoryItem::SpawnWorldItem()
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return false;
    }

    //GetTypedOuter
    AZ1Character* Character = GetTypedOuter<AZ1Character>();
    if (!IsValid(Character))
    {
        return false;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = Character;
    SpawnParams.Instigator = Character;

    FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 50.0f;
    FRotator SpawnRotation = FRotator::ZeroRotator;
    
    AZWorldItem* WorldItem = World->SpawnActor<AZWorldItem>(GetStaticData()->WorldItemClass, SpawnLocation, SpawnRotation, SpawnParams);
    if (IsValid(WorldItem))
    {
        //LOG
        //BeginDestroy();
        return true;
    }
    return false;
}

UMaterialInstanceDynamic* UZInventoryItem::CreateItemIconDynamicMaterial()
{
    FItemStaticData* ItemData = GetStaticData();
    if (ItemData)
    {
        TObjectPtr<UMaterialInstance> ItemIconInstance = ItemData->IconMaterialInstance;
        if (IsValid(ItemIconInstance))
        {
            //Outer == this? or GetWorld()?
            UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(ItemIconInstance, this);
            return DynamicMaterial;
        }
    }
    return nullptr;
}
