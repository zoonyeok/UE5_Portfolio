// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ZPotionInventoryItem.h"
#include "Z1Character.h"
#include "Components/ZAttributeComponent.h"
#include "Items/ZPotionWorldItem.h"

UZPotionInventoryItem::UZPotionInventoryItem()
{
	HealthAmount = 50;
}

void UZPotionInventoryItem::UseItem_Implementation(APawn* User)
{
	if (!IsValid(User))
	{
		// LOG: Invalid user
		return;
	}

	AZ1Character* Character = Cast<AZ1Character>(User);
	if (!IsValid(Character))
	{
		// LOG: User is not a valid AZ1Character
		return;
	}

	UZAttributeComponent* AttributeComp = Character->FindComponentByClass<UZAttributeComponent>();
	if (!IsValid(AttributeComp))
	{
		// LOG: AttributeComponent not found
		return;
	}

	// Attempt to change health
	bool ItemResult = AttributeComp->ChangeCurrentHP(this, HealthAmount);
	if (ItemResult)
	{
		// LOG: Health successfully changed
	}
	else
	{
		// LOG: Failed to change health
	}
}

EInventoryActionResult UZPotionInventoryItem::DropItem_Implementation(APawn* Dropper)
{
	if (!IsValid(Dropper))
	{
		return EInventoryActionResult::Fail;
	}

	UWorld* World = Dropper->GetWorld();
	AZ1Character* Character = Cast<AZ1Character>(Dropper);

	if (!IsValid(World) || !IsValid(Character))
	{
		// LOG: Invalid World or Dropper
		return EInventoryActionResult::Fail;
	}

	FItemStaticData* PotionStaticData = GetStaticData();
	if (!PotionStaticData)
	{
		// LOG: Invalid PotionStaticData
		return EInventoryActionResult::StaticDataLoadFail;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Character;
	SpawnParams.Instigator = Character;

	// Adjust spawn location slightly in front of the character
	FVector SpawnLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * 50.0f;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	AZPotionWorldItem* PotionWorldItem = World->SpawnActor<AZPotionWorldItem>(PotionStaticData->WorldItemClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!IsValid(PotionWorldItem))
	{
		// LOG: Failed to spawn AZPotionWorldItem
		return EInventoryActionResult::ItemSpawnFail;
	}

	// TODO
	PotionWorldItem->InitializeItem(PotionStaticData->ItemID);
	return EInventoryActionResult::DropSuccess;
}