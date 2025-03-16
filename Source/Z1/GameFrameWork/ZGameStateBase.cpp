// Fill out your copyright notice in the Description page of Project Settings.


#include "GameFrameWork/ZGameStateBase.h"
#include "Items/ZInventoryItem.h"
#include "Items/ZWorldItem.h"

bool AZGameStateBase::SpawnWorldItem(UZInventoryItem* Item, AActor* Actor, bool bGroundClamp)
{
	if (!IsValid(Item) || !IsValid(Actor))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Item or Actor passed to SpawnWorldItem."));
		return false;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid World context in SpawnWorldItem."));
		return false;
	}

	// 스폰 위치 및 회전 설정
	FVector SpawnLocation = Actor->GetActorLocation() + Actor->GetActorForwardVector() * 150.0f;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	if (bGroundClamp)
	{
		FHitResult HitResult;
		const FVector TraceEnd = SpawnLocation - FVector(0.0f, 0.0f, 1000.0f); // 아래로 1000 단위 트레이스
		bool bHit = World->LineTraceSingleByChannel(
			HitResult,
			SpawnLocation,
			TraceEnd,
			ECollisionChannel::ECC_Visibility
		);

		if (bHit)
		{
			SpawnLocation = HitResult.Location;
			SpawnLocation.Z += 100;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("GroundClamp failed to find a surface below."));
		}
	}

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// 아이템 데이터 확인
	const FItemStaticData* ItemData = Item->GetStaticData();
	if (!ItemData || !IsValid(ItemData->WorldItemClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ItemStaticData or WorldItemClass in SpawnWorldItem."));
		return false;
	}

	// WorldItem 스폰
	AZWorldItem* WorldItem = World->SpawnActor<AZWorldItem>(ItemData->WorldItemClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (IsValid(WorldItem))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully spawned WorldItem: %s at Location: %s"), *WorldItem->GetName(), *SpawnLocation.ToString());
		return true;
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to spawn WorldItem."));
	return false;
}

AZWorldItem* AZGameStateBase::SpawnWorldItemToCharacter(UZInventoryItem* Item, APawn* Pawn)
{
	if (!IsValid(Item) || !IsValid(Pawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid Item or Actor passed to SpawnWorldItem."));
		return nullptr;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid World context in SpawnWorldItem."));
		return nullptr;
	}

	// 스폰 위치 및 회전 설정
	FVector SpawnLocation = Pawn->GetActorLocation() + Pawn->GetActorForwardVector() * 150.0f;
	FRotator SpawnRotation = FRotator::ZeroRotator;

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Pawn;

	// 아이템 데이터 확인
	const FItemStaticData* ItemData = Item->GetStaticData();
	if (!ItemData || !IsValid(ItemData->WorldItemClass))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ItemStaticData or WorldItemClass in SpawnWorldItem."));
		return nullptr;
	}

	// WorldItem 스폰
	AZWorldItem* WorldItem = World->SpawnActor<AZWorldItem>(ItemData->WorldItemClass, SpawnLocation, SpawnRotation, SpawnParams);
	if (IsValid(WorldItem))
	{
		UE_LOG(LogTemp, Log, TEXT("Successfully spawned WorldItem: %s at Location: %s"), *WorldItem->GetName(), *SpawnLocation.ToString());
		return WorldItem;
	}

	UE_LOG(LogTemp, Warning, TEXT("Failed to spawn WorldItem."));
	return nullptr;
}
