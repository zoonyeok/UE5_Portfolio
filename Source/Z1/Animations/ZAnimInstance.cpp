// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/ZAnimInstance.h"

#include "AsyncTreeDifferences.h"
#include "Z1Character.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/ZPlayerMovementComponent.h"
#include "Components/ZActionComponent.h"

void UZAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AZ1Character>(TryGetPawnOwner());
	if (Character)
	{
		OwningMovementComponent = Cast<UZPlayerMovementComponent>(Character->GetComponentByClass(UZPlayerMovementComponent::StaticClass()));
		OwningSMComponent = Cast<UZActionComponent>(Character->GetComponentByClass(UZActionComponent::StaticClass()));
	} 


 }

void UZAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	/*check(Character);
	check(OwningMovementComponent);
	check(OwningSMComponent);*/

	if (!IsValid(Character) || !IsValid(OwningMovementComponent) || !IsValid(OwningSMComponent))
	{
		return;
	}

	GetGroundSpeed();
	GetAirSpeed();
	GetShouldMove();
	GetIsFalling();
	GetPitchRollYaw();
	GetYawDelta(DeltaSeconds);
	GetIsAccelerating();
	GetIsAttacking();
	GetIsFullBody();
	GetIsInAir();
	GetIsClimbing();
	//MakeTurnSide();
	//GetIsRolling();
}

void UZAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	
}

void UZAnimInstance::SetDodgeState(bool state)
{
	bCanDodge = state;
}

void UZAnimInstance::SetAnimationLayerByWeapon(TSubclassOf<UAnimInstance> WeaponAnimLayer)
{
	if (IsValid(WeaponAnimLayer))
	{
		USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
		CharacterMesh->LinkAnimClassLayers(WeaponAnimLayer);
		//UE_LOG(LogTemp, Log, TEXT("Weapon animation layer applied."));
	}
}

void UZAnimInstance::PlayAnimMontage(UAnimMontage* EquipMontage)
{
	if (IsValid(EquipMontage))
	{
		Montage_Play(EquipMontage);
		//UE_LOG(LogTemp, Log, TEXT("Equip animation montage played."));
	}
}

void UZAnimInstance::PlayComboMontage(FName NextComboName, const TObjectPtr<UAnimMontage>& ComboMontage)
{
	if (!Montage_IsPlaying(ComboMontage))
	{
		Montage_Play(ComboMontage);
	}
	
	Montage_JumpToSection(NextComboName);
}

void UZAnimInstance::GetGroundSpeed()
{
	GroundSpeed = UKismetMathLibrary::VSizeXY(Character->GetVelocity());
}

void UZAnimInstance::GetAirSpeed()
{
	AirSpeed = Character->GetVelocity().Z;
}

void UZAnimInstance::GetShouldMove()
{
	bShouldMove =
		Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 9 &&
		GroundSpeed > 5.f &&
		!bIsFalling;
}

void UZAnimInstance::GetIsFalling()
{
	bIsFalling = OwningMovementComponent->IsFalling();
}

void UZAnimInstance::GetPitchRollYaw()
{
	Pitch = Character->GetBaseAimRotation().Pitch;
	Roll = Character->GetBaseAimRotation().Roll;
	Yaw = Character->GetBaseAimRotation().Yaw;
}

void UZAnimInstance::GetYawDelta(float DeltaTime)
{
}

void UZAnimInstance::GetIsAccelerating()
{
	Character->GetCharacterMovement()->GetCurrentAcceleration().Length() > 0.f ? bIsAccelerating = true : bIsAccelerating = false;
}

void UZAnimInstance::GetIsAttacking()
{
}

void UZAnimInstance::GetIsFullBody()
{
}

void UZAnimInstance::GetIsInAir()
{
	bIsInAir = OwningMovementComponent->IsFalling();
}

void UZAnimInstance::GetIsClimbing()
{
	
}

void UZAnimInstance::MakeTurnSide()
{
	if (!Character || !Character->GetCharacterMovement())
	{
		UE_LOG(LogTemp, Warning, TEXT("Character or Character Movement is null."));
		return;
	}

	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(Character->GetActorRotation(), Character->GetControlRotation());

	if (DeltaRotation.Yaw > 90.0f)
	{
		bTurnRight = true;
		bTurnLeft = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else if (DeltaRotation.Yaw < -90.0f)
	{
		bTurnLeft = true;
		bTurnRight = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = true;
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	}
	else if (FMath::IsNearlyZero(DeltaRotation.Yaw))
	{
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true; // Depending on your needs
		bTurnLeft = false;
		bTurnRight = false;
	}
	else
	{
		// Handling the case where Yaw is exactly 90 or -90
		bTurnLeft = false;
		bTurnRight = false;
		Character->GetCharacterMovement()->bUseControllerDesiredRotation = false;
		Character->GetCharacterMovement()->bOrientRotationToMovement = true;
	}
}

void UZAnimInstance::GetIsRolling()
{
	
}
