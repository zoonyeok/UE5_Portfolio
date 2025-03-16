// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ZBaseProjectile.h"
#include "ZRangedAttackProjectile.generated.h"

/**
 * 
 */
UCLASS()
class Z1_API AZRangedAttackProjectile : public AZBaseProjectile
{
	GENERATED_BODY()

public:
	AZRangedAttackProjectile();

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageAmount;
	
	UFUNCTION()
	void OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
