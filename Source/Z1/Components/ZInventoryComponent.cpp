// Fill out your copyright notice in the Description page of Project Settings.


#include "ZInventoryComponent.h"
#include "Weapons/ZBaseWeapon.h"
#include "Z1Character.h"
#include "Weapons/ZMeleeWeaponBase.h"
#include "Weapons/ZRangedWeaponBase.h"
#include "Items/ZInventoryItem.h"

// Sets default values for this component's properties
UZInventoryComponent::UZInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	
}

// Called when the game starts
void UZInventoryComponent::BeginPlay()
{
    Super::BeginPlay();


    //TODO 임시코드
    /*if (!IsValid(GetWorld()))
    {
        return;
    }*/
    /*TObjectPtr<ACharacter> Character = Cast<ACharacter>(GetOwner());
    if (IsValid(Character))
    {
        auto CurrentWeapon = GetWorld()->SpawnActor<AZMeleeWeaponBase>();
        if (IsValid(CurrentWeapon))
        {
            FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
            CurrentWeapon->AttachToComponent(Character->GetMesh(), AttachmentRules, CurrentWeapon->GetWeaponSocketName());
        }
    }*/
}


bool UZInventoryComponent::AddItem(UZInventoryItem* Item)
{
    if (!IsValid(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid item passed to AddItem"));
        return false;
    }
    
    FItemStaticData* ItemData = Item->GetStaticData();
    FName ItemID = ItemData->ItemID;

    // 아이템이 인벤토리에 이미 있는지 확인
    if (InventoryItems.Contains(ItemID))
    {
        UE_LOG(LogTemp, Log, TEXT("Item with ID: %s already exists in the inventory"), *ItemID.ToString());
        return false;
    }

    // 인벤토리에 아이템 추가
    InventoryItems.Add(ItemID, Item);
    UE_LOG(LogTemp, Log, TEXT("Added item with ID: %s, Name: %s to inventory"), *ItemID.ToString(), *ItemData->ItemName);
    return true;
}

bool UZInventoryComponent::RemoveItem(UZInventoryItem* Item)
{
    if (!IsValid(Item))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid item passed to RemoveItem"));
        return false;
    }

    FItemStaticData* ItemData = Item->GetStaticData();
    FName ItemID = ItemData->ItemID;

    // 아이템이 인벤토리 내에 존재하는지 확인
    if (InventoryItems.Contains(ItemID))
    {
        // 아이템 제거 및 로그 출력
        TObjectPtr<UZInventoryItem> RemovedItem = InventoryItems.FindAndRemoveChecked(ItemID);

        FName RemovedItemId = RemovedItem.Get()->GetStaticData()->ItemID;
        UE_LOG(LogTemp, Log, TEXT("Removed item with ID: %s"), *RemovedItemId.ToString());
        return true;
    }

    UE_LOG(LogTemp, Warning, TEXT("Item with ID: %s does not exist in inventory"), *ItemID.ToString());
    return false;
}

bool UZInventoryComponent::AddItemToWeaponSlot(UZInventoryItem* NewItem, int32 SlotIndex)
{
    if (!IsValid(NewItem) || SlotIndex < 0 || SlotIndex >= WeaponSlots.Num())
    {
        return false; // 유효하지 않은 아이템이나 슬롯 인덱스 처리
    }

    FItemStaticData* ItemData = NewItem->GetStaticData();
    
    FGameplayTagQuery Query = FGameplayTagQuery::BuildQuery(
        FGameplayTagQueryExpression()
        .AnyTagsMatch()
        .AddTag(FGameplayTag::RequestGameplayTag(FName("Item.Weapon.Ranged")))
        .AddTag(FGameplayTag::RequestGameplayTag(FName("Item.Weapon.Melee")))
    );

    if (ItemData->ItemCategory.MatchesQuery(Query))
    {
        WeaponSlots[SlotIndex] = NewItem;
        return true;
    }

    return false;
}

TArray<UZInventoryItem*> UZInventoryComponent::GetFilteredItems(const FGameplayTag& FilterCategory)
{
    TArray<UZInventoryItem*> FilteredItems;

    for (const auto& Item : InventoryItems)
    {
        if (IsValid(Item.Value))
        {
            FItemStaticData* ItemData = Item.Value.Get()->GetStaticData();
            if (ItemData && ItemData->ItemCategory.HasTag(FilterCategory))
            {
                FilteredItems.Add(Item.Value);
            }
        }
    }

    return FilteredItems;
}


/*
*  GetMesh()->LinkAnimClassLayer(AnimLayerClass);
*/