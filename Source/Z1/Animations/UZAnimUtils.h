// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Animation/AnimSequence.h"
#include "UZAnimUtils.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API UUZAnimUtils : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Animation")
	static FTransform GetBoneTransformFromAnim(UAnimSequence* AnimSequence, FName BoneName, float Time);
	
	static FTransform LerpTransform(const FTransform& A, const FTransform& B, float Alpha);
};
