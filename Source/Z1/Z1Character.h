// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZGameTypes.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Z1Character.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class UZInventoryComponent;
class UZMeleeWeaponComponent;
class UZPlayerMovementComponent;
struct FInputActionValue;
class UAnimMontage;
class UZActionComponent;
class UZInteractComponent;
class UZSpatialInventoryComponent;
class UZEquipmentManagerComponent;
class AZBaseWeapon;
class UZAttributeComponent;
class UZInventoryItem;
class UZTargetLockComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class AZ1Character : public ACharacter
{
	GENERATED_BODY()

public:
	AZ1Character();

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	FORCEINLINE FName GetWeaponSocketParentBoneName() const { return WeaponSocketParentBoneName; }

	UFUNCTION(BlueprintCallable, Category = "Movement")
	float GetMovementDirection();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	double GetYawFromControlAndCharacterRotation();

	virtual void PossessedBy(AController* NewController) override;

	FORCEINLINE UInputMappingContext* GetMappingContext() const { return DefaultMappingContext; }

	FORCEINLINE class UZInteractComponent* GetInteractionComponent() const { return InteractComponent; }

	UPROPERTY()
	bool bCanCombo = false;

	TObjectPtr<UZInventoryItem> GetCurrentWeapon(EItemSlotType ItemSlot) const;

	FName GetCurrentWeaponSocketName(EItemSlotType ItemSlot) const;

	TPair<FTransform, FTransform> GetWeaponSocketTransform();

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZInventoryComponent> InventoryComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZMeleeWeaponComponent> MeleeWeaponComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZPlayerMovementComponent> PlayerMovementComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZActionComponent> ActionComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZTargetLockComponent> TargetLockComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZInteractComponent> InteractComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZSpatialInventoryComponent> SpatialInventoryComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZEquipmentManagerComponent> EquipmentComponent;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UZAttributeComponent> AttributeComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Weapons)
	FName WeaponSocketParentBoneName = "hand_r";

	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// To add mapping context
	virtual void BeginPlay() override;

private:
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	/** Attack Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LComboAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RComboAttack;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* RollAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LockOnAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* XButtonAction;
	
	void RollStart();
	void DodgeStart();

	void JumpStart();
	void JumpStop();

	UFUNCTION(BlueprintCallable, Category = Action)
	void LComboAttackStart();
	UFUNCTION(BlueprintCallable, Category = Action)
	void RComboAttackStart();

	void StartInteracting();
	void StopInteracting();

	void LockOn();

	void XButtonActionStart();
};
