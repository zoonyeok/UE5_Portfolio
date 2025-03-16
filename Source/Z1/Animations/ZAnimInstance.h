// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Interfaces/ZPlayComboMontage.h"
#include "ZAnimInstance.generated.h"

class UZPlayerMovementComponent;
class AZ1Character;
class UZActionComponent;
/**
 * 
 */
UCLASS()
class Z1_API UZAnimInstance : public UAnimInstance, public IZPlayComboMontage
{
	GENERATED_BODY()
	
public:
	virtual void NativeInitializeAnimation() override;

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeBeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void SetDodgeState(bool state);

	UFUNCTION(BlueprintCallable)
	UZActionComponent* GetSMComponent() const { return OwningSMComponent; };

	void SetAnimationLayerByWeapon(TSubclassOf<UAnimInstance> WeaponAnimLayer);
	void PlayAnimMontage(UAnimMontage* EquipMontage);

	void PlayComboMontage(FName NextComboName, const TObjectPtr<UAnimMontage>& ComboMontage);

private:

	UPROPERTY()
	TObjectPtr<AZ1Character> Character;

	UPROPERTY()
	TObjectPtr<UZPlayerMovementComponent> OwningMovementComponent;

	UPROPERTY()
	TObjectPtr<UZActionComponent> OwningSMComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = WeaponAnimation, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UAnimInstance> DefaultAnimLayer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float GroundSpeed;
	void GetGroundSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float AirSpeed;
	void GetAirSpeed();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bShouldMove : 1;
	void GetShouldMove();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bIsFalling : 1;
	void GetIsFalling();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float Pitch;
	void GetPitchRollYaw();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float Roll;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float Yaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float YawDelta;
	void GetYawDelta(float DeltaTime);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	FRotator RotationLastTick;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bIsAccelerating : 1;
	void GetIsAccelerating();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bIsAttacking : 1;
	void GetIsAttacking();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bIsFullBody : 1;
	void GetIsFullBody();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bIsInAir : 1;
	void GetIsInAir();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	uint8 bIsClimbing : 1;
	void GetIsClimbing();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Reference, meta = (AllowPrivateAccess = "true"))
	float MovementDirection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	uint8 bTurnRight : 1;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	uint8 bTurnLeft : 1;
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void MakeTurnSide();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	uint8 bIsRolling : 1;
	void GetIsRolling();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	uint8 bCanDodge : 1;

	UPROPERTY()
	FName LightComboAttack;
	UPROPERTY()
	FName HeavyComboAttack;
	UPROPERTY()
	FName DefaultLightComboAttack;
	UPROPERTY()
	FName DefaultHeavyComboAttack;
};
