// Copyright Epic Games, Inc. All Rights Reserved.

#include "Z1Character.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/ZPlayerMovementComponent.h"
#include "Components/ZActionComponent.h"
#include "Components/ZInteractComponent.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "Components/ZAttributeComponent.h"
#include "ZGameplayTags.h"
#include "Actions/ZNotifyComboAction.h"
#include "Actions/ZComboAction.h"
#include "Actions/ZMoveAction.h"
#include "Animation/AnimData/IAnimationDataModel.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "Animations/ZAnimInstance.h"
#include "Components/ZTargetLockComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Items/ZInventoryItem.h"
// #include "Weapons/ZMeleeWeaponComponent.h"
// #include "Weapons/ZBaseWeapon.h"
// #include "Components/ZInventoryComponent.h"
// #include "UI/WorldItemWidgetComponent.h"
// #include "Items/ZInventoryItem.h"
// #include "Items/ZWorldItem.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AZ1Character

AZ1Character::AZ1Character()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	SetRootComponent(GetCapsuleComponent());

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 0.0f, 88.0f)); // 캐릭터 몸 위쪽으로 배치

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	FollowCamera->SetRelativeLocation(FVector::ZeroVector);

	GetMesh()->SetRelativeLocation(FVector(0.0f, 0.0f, -88.0f)); // 캡슐과 정렬

	//MeleeWeaponComponent = CreateDefaultSubobject<UZMeleeWeaponComponent>(TEXT("MeleeWeaponComp"));
	//InventoryComponent = CreateDefaultSubobject<UZInventoryComponent>(TEXT("InventoryComp"));
	ActionComponent = CreateDefaultSubobject<UZActionComponent>(TEXT("ActionComp"));
	PlayerMovementComponent = CreateDefaultSubobject<UZPlayerMovementComponent>(TEXT("MovementComp"));
	InteractComponent = CreateDefaultSubobject<UZInteractComponent>(TEXT("InteractComp"));
	SpatialInventoryComponent = CreateDefaultSubobject<UZSpatialInventoryComponent>(TEXT("SpatialInventoryComponent"));
	EquipmentComponent = CreateDefaultSubobject<UZEquipmentManagerComponent>(TEXT("EquipmentComponent"));
	AttributeComponent = CreateDefaultSubobject<UZAttributeComponent>(TEXT("AttributeComponent"));
	TargetLockComponent = CreateDefaultSubobject<UZTargetLockComponent>(TEXT("TargetLockOnComponent"));
}

void AZ1Character::BeginPlay()
{
	Super::BeginPlay();

	//UE_LOG(LogTemplateCharacter, Warning, TEXT("Root Component Name: %s"), *GetRootComponent()->GetName());
	//UE_LOG(LogTemplateCharacter, Warning, TEXT("CameraBoom Location: %s"), *CameraBoom->GetComponentLocation().ToString());
	//UE_LOG(LogTemplateCharacter, Warning, TEXT("FollowCamera Location: %s"), *FollowCamera->GetComponentLocation().ToString());

	check(ActionComponent);
	check(PlayerMovementComponent);
	check(InteractComponent);
	check(SpatialInventoryComponent);
	check(EquipmentComponent);
	check(AttributeComponent);
	check(TargetLockComponent);
}

//////////////////////////////////////////////////////////////////////////
// Input

void AZ1Character::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ThisClass::JumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ThisClass::JumpStop);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

		// Attack
		EnhancedInputComponent->BindAction(LComboAttack, ETriggerEvent::Triggered, this, &ThisClass::LComboAttackStart);
		EnhancedInputComponent->BindAction(RComboAttack, ETriggerEvent::Triggered, this, &ThisClass::RComboAttackStart);

		// Rolling
		EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &ThisClass::RollStart);

		// Dodging
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ThisClass::DodgeStart);

		// Climb
		//Interaction
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this,
			&ThisClass::StartInteracting);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &ThisClass::StopInteracting);

		// LockOn
		EnhancedInputComponent->BindAction(LockOnAction, ETriggerEvent::Triggered, this, &ThisClass::LockOn);

		// XButton
		EnhancedInputComponent->BindAction(XButtonAction, ETriggerEvent::Triggered, this,
			&ThisClass::XButtonActionStart);
	}
}

TObjectPtr<UZInventoryItem> AZ1Character::GetCurrentWeapon(EItemSlotType ItemSlot) const
{
	if (ItemSlot == EItemSlotType::EIS_Weapon_1 || ItemSlot == EItemSlotType::EIS_Weapon_2)
	{
		if (TObjectPtr<UZInventoryItem> CurrentWeapon = EquipmentComponent->GetEquippedItem(ItemSlot))
		{
			return CurrentWeapon;
		}
	}
	return nullptr;
}

FName AZ1Character::GetCurrentWeaponSocketName(EItemSlotType ItemSlot) const
{
	if (TObjectPtr<UZInventoryItem> CurrentWeapon = GetCurrentWeapon(ItemSlot))
	{
		return CurrentWeapon->GetStaticData()->ItemSocketName;
	}
	return FName(NAME_None);
}

TPair<FTransform, FTransform> AZ1Character::GetWeaponSocketTransform()
{
	FTransform StartTransform;
	FTransform EndTransform;

	const TArray<TObjectPtr<USceneComponent>>& SceneComponents = GetMesh()->GetAttachChildren();
	for (const auto& SceneComp : SceneComponents)
	{
		if (IsValid(SceneComp))
		{
			StartTransform = SceneComp->GetSocketTransform(FName("KatanaSocket_Start"));
			EndTransform = SceneComp->GetSocketTransform(FName("KatanaSocket_End"));
			return TPair<FTransform, FTransform>(StartTransform, EndTransform);
		}
	}

	return TPair<FTransform, FTransform>(StartTransform, EndTransform);
}

void AZ1Character::Move(const FInputActionValue& Value)
{
	if (TObjectPtr<UZMoveAction> MovementAction = Cast<UZMoveAction>(ActionComponent->FindActionByTag(ZGameplayTags::Action_Movement_Moving)))
	{
		if (ActionComponent->StartActionByTag(this, ZGameplayTags::Action_Movement_Moving))
		{
			//MovementAction->Move(Value);

			// 입력 값 가져오기
			FVector2D MovementVector = Value.Get<FVector2D>();

			// 현재 캐릭터의 회전 가져오기
			const FRotator Rotation = GetController()->GetControlRotation();
			const FRotator YawRotation(0, Rotation.Yaw, 0);

			// 앞/옆 이동 벡터 계산
			const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
			const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

			// 이동 입력 적용
			AddMovementInput(ForwardDirection, MovementVector.Y);
			AddMovementInput(RightDirection, MovementVector.X);
		}

		ActionComponent->StopActionByTag(this, ZGameplayTags::Action_Movement_Moving);
	}
}

void AZ1Character::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (IsValid(Controller))
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

float AZ1Character::GetMovementDirection()
{
	if (GetVelocity().IsZero())
	{
		return 0.0f;
	}

	const auto VelocityNormal = GetVelocity().GetSafeNormal();
	const auto AngleBetween = FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
	const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);
	const auto Degrees = FMath::RadiansToDegrees(AngleBetween);

	float ret = CrossProduct.IsZero() ? Degrees : Degrees * FMath::Sign(CrossProduct.Z);
	//UE_LOG(LogTemp, Log, TEXT("Direction Value is: %f"), ret);
	return ret;
}

double AZ1Character::GetYawFromControlAndCharacterRotation()
{
	FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetActorRotation(), GetControlRotation());
	//UE_LOG(LogTemp, Log, TEXT("Diff Value is: %f"), DeltaRotation.Yaw);
	return DeltaRotation.Yaw;
}

void AZ1Character::PossessedBy(AController* NewController)
{
	// TODO
	/*
	* ZPlayerState에서 불러온 스탯을 UZAttributeComponent에 대입해줘야 하는데, 위 시점에서 UZAttributeComponent생성되지 않았다면?
	* `APawn::PossessedBy`에서 데이터를 로드
	*/
}

void AZ1Character::RollStart()
{
	ActionComponent->StartActionByTag(this, ZGameplayTags::Action_Movement_Rolling);
}

void AZ1Character::DodgeStart()
{
	ActionComponent->StartActionByTag(this, ZGameplayTags::Action_Movement_Dodging);
}

void AZ1Character::JumpStart()
{
	ActionComponent->StartActionByTag(this, ZGameplayTags::Action_Movement_Jumping);
}

void AZ1Character::JumpStop()
{
}

void AZ1Character::LComboAttackStart()
{
	if (!ActionComponent->StartActionByTag(this, ZGameplayTags::Action_Attack_LCombo))
	{
		UZNotifyComboAction* ComboAction = Cast<UZNotifyComboAction>(
			ActionComponent->FindActionByTag(ZGameplayTags::Action_Attack_LCombo));
		if (ComboAction && ComboAction->IsRunning())
		{
			ComboAction->ProcessComboCommand();
		}
	}
}

void AZ1Character::RComboAttackStart()
{
	if (!ActionComponent->StartActionByTag(this, ZGameplayTags::Action_Attack_RCombo))
	{
		UZNotifyComboAction* ComboAction = Cast<UZNotifyComboAction>(
			ActionComponent->FindActionByTag(ZGameplayTags::Action_Attack_RCombo));
		if (ComboAction && ComboAction->IsRunning())
		{
			ComboAction->ProcessComboCommand();
		}
	}
}

void AZ1Character::StartInteracting()
{
	InteractComponent->BeginInteract();
}

void AZ1Character::StopInteracting()
{
	InteractComponent->EndInteract();
}

void AZ1Character::LockOn()
{
	UE_LOG(LogTemp, Display, TEXT("LockOn Target"));

	TargetLockComponent->ToggleLockOnTarget();
}

void AZ1Character::XButtonActionStart()
{
	if (IsValid(ActionComponent))
	{
		ActionComponent->StartActionByTag(this, ZGameplayTags::Controller_Button_X);
	}
}
