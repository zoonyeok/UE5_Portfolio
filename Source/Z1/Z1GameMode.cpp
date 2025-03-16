// Copyright Epic Games, Inc. All Rights Reserved.

#include "Z1GameMode.h"
#include "UObject/ConstructorHelpers.h"
#include "ZSaveGameSubsystem.h"
#include <Kismet/GameplayStatics.h>
#include "EngineUtils.h"
#include "AI/ZAICharacter.h"
#include "Components/ZAttributeComponent.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "GameFrameWork/ZGameStateBase.h"

AZ1GameMode::AZ1GameMode()
{
	// set default pawn class to our Blueprinted character
	/*static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}*/

	SpawnTimerInterval = 2.0f;
	MaxFPS = 30.f;
}

void AZ1GameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	//
	// (Save/Load logic moved into new SaveGameSubsystem)
	UZSaveGameSubsystem* SG = GetGameInstance()->GetSubsystem<UZSaveGameSubsystem>();

	// Optional slot name (Falls back to slot specified in SaveGameSettings class/INI otherwise)
	FString SelectedSaveSlot = UGameplayStatics::ParseOption(Options, "SaveGame");
	SG->LoadSaveGame(SelectedSaveSlot);
}

void AZ1GameMode::StartPlay()
{
	Super::StartPlay();

	GEngine->SetMaxFPS(MaxFPS);

	GetWorldTimerManager().SetTimer(TimerHandle_SpawnBots, this, &AZ1GameMode::SpawnBotTimerElapse, SpawnTimerInterval, true);
}

void AZ1GameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	// Calling Before Super:: so we set variables before 'beginplayingstate' is called in PlayerController (which is where we instantiate UI)
	UZSaveGameSubsystem* SG = GetGameInstance()->GetSubsystem<UZSaveGameSubsystem>();
	SG->HandleStartingNewPlayer(NewPlayer);

	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	// Now we're ready to override spawn location
	// Alternatively we could override core spawn location to use store locations immediately (skipping the whole 'find player start' logic)
	SG->OverrideSpawnTransform(NewPlayer);
}

void AZ1GameMode::SpawnBotTimerElapse()
{
	int32 NumberOfAliveBots = 0;
	for (TActorIterator<AZAICharacter> It(GetWorld()); It; ++It)
	{
		if (TObjectPtr <AZAICharacter> Bot = *It)
		{
			TObjectPtr<UZAttributeComponent> BotAttributeComp = Bot->GetComponentByClass<UZAttributeComponent>();
			if (IsValid(BotAttributeComp) && BotAttributeComp->IsAlive())
			{
				NumberOfAliveBots++;
			}
		}
	}
	
	if (IsValid(DifficultyCurve))
	{
		float CurrentTime = GetWorld()->GetTimeSeconds();
		float DifficultyValue = DifficultyCurve->GetFloatValue(CurrentTime);

		UE_LOG(LogTemp, Warning, TEXT("Current Server Time: %f, Difficulty Value: %f"), CurrentTime, DifficultyValue);
        
		MaxBotCount = static_cast<int32>(DifficultyValue);
	}
	else
	{
		UE_LOG(LogTemp, Display, TEXT("DifficultyCurve is NULL"));
	}

	if (NumberOfAliveBots >= MaxBotCount)
	{
		return;
	}
	
	TObjectPtr<UEnvQueryInstanceBlueprintWrapper> SpawnEnvQueryInstance = UEnvQueryManager::RunEQSQuery(this, SpawnBotQuery, this, EEnvQueryRunMode::RandomBest5Pct, nullptr);
	if (IsValid(SpawnEnvQueryInstance))
	{
		SpawnEnvQueryInstance->GetOnQueryFinishedEvent().AddDynamic(this, &AZ1GameMode::OnSpawnQueryCompleted);
	}
}

void AZ1GameMode::OnSpawnQueryCompleted(UEnvQueryInstanceBlueprintWrapper* QueryInstance, EEnvQueryStatus::Type QueryStatus)
{
	if (QueryStatus != EEnvQueryStatus::Success)
	{
		UE_LOG(LogTemp, Display, TEXT("OnSpawnQuery Failed"));
		return;
	}
	
	TArray<FVector> ActorLocations = QueryInstance->GetResultsAsLocations();
	if (ActorLocations.IsValidIndex(0))
	{
		GetWorld()->SpawnActor<AActor>(MinionClass, ActorLocations[0], FRotator::ZeroRotator);
	}
}