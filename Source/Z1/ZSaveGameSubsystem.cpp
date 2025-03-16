// Fill out your copyright notice in the Description page of Project Settings.


#include "ZSaveGameSubsystem.h"

#include "Z1.h"
#include "EngineUtils.h"
//#include "SGameplayInterface.h"
#include "ZPlayerState.h"
#include "ZSaveGame.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"


#include UE_INLINE_GENERATED_CPP_BY_NAME(ZSaveGameSubsystem)

void UZSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
}


void UZSaveGameSubsystem::HandleStartingNewPlayer(AController* NewPlayer)
{
	/*AZPlayerState* PS = NewPlayer->GetPlayerState<AZPlayerState>();
	if (ensure(PS))
	{
		PS->LoadPlayerState(CurrentSaveGame);
	}*/
}

bool UZSaveGameSubsystem::OverrideSpawnTransform(AController* NewPlayer)
{
	if (!IsValid(NewPlayer))
	{
		return false;
	}

	APlayerState* PS = NewPlayer->GetPlayerState<APlayerState>();
	if (PS == nullptr)
	{
		return false;
	}

	if (APawn* MyPawn = PS->GetPawn())
	{
		FPlayerSaveData* FoundData = CurrentSaveGame->GetPlayerData(PS);
		if (FoundData && FoundData->bResumeAtTransform)
		{
			MyPawn->SetActorLocation(FoundData->Location);
			MyPawn->SetActorRotation(FoundData->Rotation);

			// PlayerState owner is a (Player)Controller
			AController* PC = CastChecked<AController>(PS->GetOwner());
			// Set control rotation to change camera direction, setting Pawn rotation is not enough
			PC->SetControlRotation(FoundData->Rotation);

			return true;
		}
	}

	return false;
}

void UZSaveGameSubsystem::SetSlotName(FString NewSlotName)
{
	// Ignore empty name
	if (NewSlotName.Len() == 0)
	{
		return;
	}

	CurrentSlotName = NewSlotName;
}

void UZSaveGameSubsystem::WriteSaveGame()
{
	// Clear arrays, may contain data from previously loaded SaveGame
	CurrentSaveGame->SavedPlayers.Empty();
	CurrentSaveGame->SavedActors.Empty();

	AGameStateBase* GS = GetWorld()->GetGameState();
	if (GS == nullptr)
	{
		// Warn about failure to save?
		return;
	}

	// Iterate all player states, we don't have proper ID to match yet (requires Steam or EOS)
	for (int32 i = 0; i < GS->PlayerArray.Num(); i++)
	{
		AZPlayerState* PS = CastChecked<AZPlayerState>(GS->PlayerArray[i]);
		if (PS)
		{
			PS->SavePlayerState(CurrentSaveGame);
			break; // single player only supported at this point
		}
	}

	// Iterate the entire world of actors
	for (FActorIterator It(GetWorld()); It; ++It)
	{
		AActor* Actor = *It;
		// Only interested in our 'gameplay actors', skip actors that are being destroyed
		// Note: You might instead use a dedicated SavableObject interface for Actors you want to save instead of re-using GameplayInterface

		// TODO USGameplayInterface
		/*if (!IsValid(Actor) || !Actor->Implements<USGameplayInterface>())
		{
			continue;
		}*/

		FActorSaveData ActorData;
		ActorData.ActorName = Actor->GetFName();
		ActorData.Transform = Actor->GetActorTransform();

		// Pass the array to fill with data from Actor
		FMemoryWriter MemWriter(ActorData.ByteData);

		FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
		// Find only variables with UPROPERTY(SaveGame)
		Ar.ArIsSaveGame = true;
		// Converts Actor's SaveGame UPROPERTIES into binary array
		Actor->Serialize(Ar);

		CurrentSaveGame->SavedActors.Add(ActorData);
	}

	UGameplayStatics::SaveGameToSlot(CurrentSaveGame, CurrentSlotName, 0);

	OnSaveGameWritten.Broadcast(CurrentSaveGame);
}

void UZSaveGameSubsystem::LoadSaveGame(FString InSlotName)
{
	// Update slot name first if specified, otherwise keeps default name
	SetSlotName(InSlotName);

	if (UGameplayStatics::DoesSaveGameExist(CurrentSlotName, 0))
	{
		CurrentSaveGame = Cast<UZSaveGame>(UGameplayStatics::LoadGameFromSlot(CurrentSlotName, 0));
		if (CurrentSaveGame == nullptr)
		{
			UE_LOGFMT(LogGame, Warning, "Failed to load SaveGame Data.");
			return;
		}

		UE_LOGFMT(LogGame, Log, "Loaded SaveGame Data.");


		// Iterate the entire world of actors
		for (FActorIterator It(GetWorld()); It; ++It)
		{
			AActor* Actor = *It;

			// TODO USGameplayInterface
			// Only interested in our 'gameplay actors'
			/*if (!Actor->Implements<USGameplayInterface>())
			{
				continue;
			}*/

			for (FActorSaveData ActorData : CurrentSaveGame->SavedActors)
			{
				if (ActorData.ActorName == Actor->GetFName())
				{
					Actor->SetActorTransform(ActorData.Transform);

					FMemoryReader MemReader(ActorData.ByteData);

					FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
					Ar.ArIsSaveGame = true;
					// Convert binary array back into actor's variables
					Actor->Serialize(Ar);

					// TODO USGameplayInterface
					//ISGameplayInterface::Execute_OnActorLoaded(Actor);

					break;
				}
			}
		}

		OnSaveGameLoaded.Broadcast(CurrentSaveGame);
	}
	else
	{
		CurrentSaveGame = CastChecked<UZSaveGame>(UGameplayStatics::CreateSaveGameObject(UZSaveGame::StaticClass()));

		UE_LOGFMT(LogGame, Log, "Created New SaveGame Data.");
	}
}

