// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZMeleeWeaponComponent.generated.h"

class AZMeleeWeaponBase;
class AZBaseWeapon;
class UZAnimInstance;
class UZComboActionData;
class UAnimMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZMeleeWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZMeleeWeaponComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetCurrentWeaponFromInventory(TObjectPtr<AZMeleeWeaponBase> Weapon);

	//void UnseatheWeapon();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// 충돌 관련 메서드
	virtual void ComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AZMeleeWeaponBase> WeaponClass;

	UPROPERTY(VisibleanyWhere, BlueprintReadWrite)
	FName MeleeWeaponSocketName;

private:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Combo, meta=(AllowPrivateAccess=true))
	TObjectPtr<AZMeleeWeaponBase> CurrentWeapon;

	void SpawnWeapon();
	
	UZAnimInstance* GetCharacterAnimInstance() const;
};
