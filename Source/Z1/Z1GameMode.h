// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTypes.h"
#include "GameFramework/GameModeBase.h"
#include "Z1GameMode.generated.h"

class UEnvQueryInstanceBlueprintWrapper;
class UEnvQuery;
class UCurveFloat;

UCLASS(minimalapi)
class AZ1GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AZ1GameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;

	virtual void StartPlay() override;

	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

protected:
	FTimerHandle TimerHandle_SpawnBots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpawnAI)
	float SpawnTimerInterval;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = SpawnAI)
	TObjectPtr<UEnvQuery> SpawnBotQuery;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = SpawnAI)
	TSubclassOf<AActor> MinionClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = SpawnAI)
	int32 MaxBotCount = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = SpawnAI)
	TObjectPtr<UCurveFloat> DifficultyCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Setting)
	float MaxFPS;
	
	void SpawnBotTimerElapse();

	UFUNCTION()
	void OnSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus);
};



