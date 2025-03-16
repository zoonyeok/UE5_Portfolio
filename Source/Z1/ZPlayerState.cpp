// Fill out your copyright notice in the Description page of Project Settings.


#include "ZPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "ZSaveGame.h"
#include "Z1.h"
#include "Components/ZAttributeComponent.h"

void AZPlayerState::InitializeCharacterStats(FCharacterStats InitialStats)
{
}

void AZPlayerState::UpdateCharacterStats(FCharacterStats FinalStats)
{
	//ZAttributeComp에서 변경된 모든 값 업데이트
	TObjectPtr<UZAttributeComponent> ACP = GetOwner()->GetComponentByClass<UZAttributeComponent>();
	if (ACP)
	{
		this->CharacterStats = FinalStats;
	}
}

void AZPlayerState::OnRep_CharacterStats(FCharacterStats RepCharacterStats)
{
}

void AZPlayerState::SavePlayerState_Implementation(UZSaveGame* SaveObject)
{
	if (SaveObject)
	{
		// Gather all relevant data for player
		FPlayerSaveData SaveData;

		// TODO 저장하기전 최신 스탯으로 업데이트 필요
		if (APawn* MyPawn = GetPawn())
		{
			if (TObjectPtr<UZAttributeComponent> ACP = GetPawn()->GetComponentByClass<UZAttributeComponent>())
			{
				SaveData.CharacterStats = ACP->GetFCharacterStats();
				// Stored as FString for simplicity (original Steam ID is uint64)
				SaveData.PlayerID = GetUniqueId().ToString();
			}
		}

		// May not be alive while we save
		if (APawn* MyPawn = GetPawn())
		{
			SaveData.Location = MyPawn->GetActorLocation();
			SaveData.Rotation = MyPawn->GetActorRotation();
			SaveData.bResumeAtTransform = true;
		}

		SaveObject->SavedPlayers.Add(SaveData);
	}
}

void AZPlayerState::LoadPlayerState_Implementation(UZSaveGame* SaveObject)
{
	if (SaveObject)
	{
		FPlayerSaveData* FoundData = SaveObject->GetPlayerData(this);
		if (FoundData)
		{
			CharacterStats = FoundData->CharacterStats;
			//TODO : ZPlayerState에서 불러온 스탯을 UZAttributeComponent에 대입
			//이 시점에 ActorComponent가 생성되어 있는지?
			//안될듯? Getter만들어?
			if (APawn* MyPawn = GetPawn())
			{
				if (TObjectPtr<UZAttributeComponent> ACP = GetPawn()->GetComponentByClass<UZAttributeComponent>())
				{
					// 스탯을 ACP에 대입
					ACP->InitializeCharacterStats(FoundData->CharacterStats);
				}
			}
		}
		else
		{
			UE_LOGFMT(LogGame, Warning, "Could not find SaveGame data for player id: {playerid}.", GetPlayerId());
		}
	}
}

FCharacterStats* AZPlayerState::GetFCharacterStatsFromDataTable(FString StatsDataTablePath, FName RowName, FString ContextString)
{
	// UDataTable을 로드
	UDataTable* CharacterDataTable = LoadObject<UDataTable>(nullptr, *StatsDataTablePath);
	if (CharacterDataTable)
	{
		// DataTable에서 특정 RowName으로 데이터를 찾음
		FCharacterStats* CharacterStatsFromTable = CharacterDataTable->FindRow<FCharacterStats>(RowName, *ContextString);
		if (CharacterStatsFromTable)
		{
			return CharacterStatsFromTable;
		}
		else
		{
			UE_LOGFMT(LogGame, Warning, "Could not find RowName:{RowName} in Context: {ContextString}.", *RowName.ToString(), *ContextString);
			//UE_LOG(LogGame, Warning, TEXT("Could not find RowName: %s in Context: %s."), *RowName.ToString(), *ContextString);
		}
	}
	else
	{
		UE_LOG(LogGame, Warning, TEXT("Could not find DataTable at path: %s."), *StatsDataTablePath);
	}

	return nullptr;
}

void AZPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AZPlayerState, CharacterStats);
}