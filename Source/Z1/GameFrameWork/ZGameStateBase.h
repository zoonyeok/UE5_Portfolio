// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "ZGameStateBase.generated.h"

/**
 * 현재 맵에 존재하는 아이템 개수는 AGameStateBase에서 관리하는 것이 좋음!
 */
UCLASS()
class Z1_API AZGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:

	// TODO : GameMode로 옮기기
	UFUNCTION(BlueprintCallable)
	bool SpawnWorldItem(UZInventoryItem* Item, AActor* Actor, bool bGroundClamp = true);

	UFUNCTION(BlueprintCallable)
	AZWorldItem* SpawnWorldItemToCharacter(UZInventoryItem* Item, APawn* Pawn);
};
