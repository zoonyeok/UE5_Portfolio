// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ZPotionWorldItem.h"
#include "Z1Character.h"
#include "ZGameTypes.h"
#include "Components/ZInventoryComponent.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "UI/WorldItemWidgetComponent.h"
#include "Items/ZPotionInventoryItem.h"

AZPotionWorldItem::AZPotionWorldItem()
{
    StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    RootComponent = StaticMeshComponent;

    // Ensure it has proper collision settings for a pickup object
    StaticMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);

    StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    StaticMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    StaticMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
    WorldItemWidgetComponent->SetupAttachment(RootComponent);
    StaticMeshComponent->SetSimulatePhysics(true);
}

EInventoryActionResult AZPotionWorldItem::PickUp_Implementation(APawn* Player)
{
    AZ1Character* Character = Cast<AZ1Character>(Player);
    if (!IsValid(Character))
    {
        // LOG
        return EInventoryActionResult::Fail;
    }

    UZSpatialInventoryComponent* InvenComp = Character->GetComponentByClass<UZSpatialInventoryComponent>();
    //UZInventoryComponent* InvenComp = Character->GetComponentByClass<UZInventoryComponent>();
    if (!IsValid(InvenComp))
    {
        // LOG
        return EInventoryActionResult::Fail;
    }

    UZPotionInventoryItem* PotionInvenItem = Cast<UZPotionInventoryItem>(ConvertToInventoryItem());
    if (PotionInvenItem)
    {
        bool ItemAddResult = false;
        ItemAddResult = InvenComp->TryAddItem(PotionInvenItem);
        if (ItemAddResult)
        {
            //TODO : Widget Update , Delegate
            this->Destroy();
            return EInventoryActionResult::Success;
        }
    }

    return EInventoryActionResult::Fail;
}

UZInventoryItem* AZPotionWorldItem::ConvertToInventoryItem()
{
    UZPotionInventoryItem* PotionInvenItem = NewObject<UZPotionInventoryItem>();
    PotionInvenItem->Initialize(ItemDataTable, ItemID, 1);

    if (PotionInvenItem)
    {
        return PotionInvenItem;
    }

    return nullptr;
}
