// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZComboActionData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class Z1_API UZComboActionData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UZComboActionData();

	UPROPERTY(EditAnywhere, Category = ComboData)
	FString MontageSectionNamePrefix;

	UPROPERTY(EditAnywhere, Category = ComboData)
	uint8 MaxComboCount;

	UPROPERTY(EditAnywhere, Category = ComboData)
	float FrameRate;

	// 사전에 입력됐는지 감지하는 프레임
	UPROPERTY(EditAnywhere, Category = ComboData)
	TArray<float> EffectiveFrameCount;
};
