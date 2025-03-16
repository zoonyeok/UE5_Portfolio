// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "ZGameTypes.h" 
#include "ZPlayerState.generated.h"

class UZSaveGame;
/**
 * 
 */
UCLASS()
class Z1_API AZPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
   
    // 캐릭터 능력치를 초기화하는 함수
    void InitializeCharacterStats(FCharacterStats InitialStats);

    // 캐릭터의 능력치를 업데이트하는 함수
    void UpdateCharacterStats(FCharacterStats FinalStats);

    /*
    void LoadCharacterStats();
    void SaveCharacterStats();
    */
    UFUNCTION(BlueprintNativeEvent)
    void SavePlayerState(UZSaveGame* SaveObject);

    UFUNCTION(BlueprintNativeEvent)
    void LoadPlayerState(UZSaveGame* SaveObject);

protected:

    UPROPERTY(EditDefaultsOnly, ReplicatedUsing = "OnRep_CharacterStats", Category = "Stats", meta = (AllowPrivateAccess = "true"))
    FCharacterStats CharacterStats;

    // OnRep_ can use a parameter containing the 'old value' of the variable it is bound to. Very useful in this case to figure out the 'delta'.
    UFUNCTION()
    void OnRep_CharacterStats(FCharacterStats RepCharacterStats);

private:

    FCharacterStats* GetFCharacterStatsFromDataTable(FString StatDataTablePath, FName RowName, FString ContextString);

    FString PlayerStatsDataTablePath = TEXT("/Game/Data/CharacterStatsTable.CharacterStatsTable");
    FName PlayerRowName = FName("Character1");  // CharacterStatsTable의 특정 행 이름
    FString PlayerContextString = TEXT("Character Stats Context");

};
