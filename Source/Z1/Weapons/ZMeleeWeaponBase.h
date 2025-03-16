// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ZBaseWeapon.h"
#include "ZMeleeWeaponBase.generated.h"

class UZWeaponTraceComponent;
/**
 * 
 */
UCLASS()
class Z1_API AZMeleeWeaponBase : public AZBaseWeapon
{
	GENERATED_BODY()

public:

	AZMeleeWeaponBase();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	USkeletalMeshComponent* GetMesh() const { return SkeletalMeshComponent; }
	
protected:

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UZWeaponTraceComponent> TraceComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void StartAttack() override;

	virtual void EndAttack() override;

	virtual EInventoryActionResult PickUp(APawn* Player);

	virtual UZInventoryItem* ConvertToInventoryItem() override;

	virtual void PostInitializeComponents() override;
	// Animnotify
	
	//// Physics Asset: 무기의 물리적 상호작용 정의
	//UPROPERTY(EditAnywhere, BlueprintReadWrite)
	//UPhysicsAsset* WeaponPhysicsAsset;

	UFUNCTION()
	void OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	float CalculateFinalDamage() override;
};
