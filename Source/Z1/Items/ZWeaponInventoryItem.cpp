// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/ZWeaponInventoryItem.h"
#include "Z1Character.h"
#include "Weapons/ZBaseWeapon.h"

UZWeaponInventoryItem::UZWeaponInventoryItem()
{
}

//TODO : 이 함수를 일반화 가능한지
// 매번 AZPotionWorldItem 이런타입을 수동으로 타이핑 하지않게
// FItemStaticData에 UZWeaponInventoryItem에 해당하는 WorldItem을 넣어버릴까
// 아님 매개변수로  UZWeaponInventoryItem에 해당하는 WorldItem을 넘겨줄까
// 그럼 DropItem 호출하는 곳이 어디? 
// 인벤토리 창에서 월드에 마우스 드래그로 버림
// 인벤토리 창에서 특정버튼으로 아이템 버림
//TSubclassof
EInventoryActionResult UZWeaponInventoryItem::DropItem_Implementation(APawn* Dropper)
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

	FItemStaticData* StaticData = GetStaticData();
	if (!StaticData)
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

	//AZWorldItem* WorldItem = World->SpawnActor<AZWorldItem>(PotionStaticData->WorldItemClass, SpawnLocation, SpawnRotation, SpawnParams);
	AZBaseWeapon* WorldItem = World->SpawnActor<AZBaseWeapon>(StaticData->WorldItemClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (!IsValid(WorldItem))
	{
		// LOG: Failed to spawn AZPotionWorldItem
		return EInventoryActionResult::ItemSpawnFail;
	}

	// TODO
	WorldItem->InitializeItem(WorldItem->GetItemID());
	return EInventoryActionResult::DropSuccess;
}
