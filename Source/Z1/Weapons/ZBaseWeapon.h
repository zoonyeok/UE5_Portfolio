// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/ZWorldItem.h"
#include "ZBaseWeapon.generated.h"

class USkeletalMeshComponent;
//class USoundCue;
UCLASS()
class Z1_API AZBaseWeapon : public AZWorldItem
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZBaseWeapon();

	TSubclassOf<UAnimInstance> GetDisarmedCharacterAnimLayer() const { return DisarmedCharacterAnimLayer; }

	TSubclassOf<UAnimInstance> GetArmedCharacterAnimLayer() const { return ArmedCharacterAnimLayer; }

	TObjectPtr<UAnimMontage> GetEquipAnimMontage() const { return EquipAnimMontage; }

	TObjectPtr<UAnimMontage> GetDisarmAnimMontage() const { return DisarmAnimMontage; }

	FName GetWeaponSocketName() const { return WeaponSocketName; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void StartAttack();

	virtual void EndAttack();

	virtual float CalculateFinalDamage();

	virtual EInventoryActionResult PickUp_Implementation(APawn* Player) override;

	virtual UZInventoryItem* ConvertToInventoryItem() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	FName WeaponSocketName = "";

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
	FName MuzzleSocketName;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Sound")
	USoundCue* FireSound;*/

	// 기타 무기 관련 데이터

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Stat)
	float AttackDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Stat)
	float FinalDamage;

	//VFX effect
	//SOUND effect

	//Weapon AnimData
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBase | AnimLayer", meta = (AllowPrivateAccess))
	TSubclassOf<UAnimInstance> DisarmedCharacterAnimLayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBase | AnimLayer", meta = (AllowPrivateAccess))
	TSubclassOf<UAnimInstance> ArmedCharacterAnimLayer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBase", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> EquipAnimMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "WeaponBase", meta = (AllowPrivateAccess))
	TObjectPtr<UAnimMontage> DisarmAnimMontage;
};
