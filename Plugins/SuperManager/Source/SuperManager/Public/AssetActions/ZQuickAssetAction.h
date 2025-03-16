// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "ZQuickAssetAction.generated.h"

/**
 * 
 */
UCLASS()
class SUPERMANAGER_API UZQuickAssetAction : public UAssetActionUtility
{
	GENERATED_BODY()
	
public:
	UFUNCTION(CallInEditor)
	void DuplicateAssets(int32 NumofDuplicates);


};
