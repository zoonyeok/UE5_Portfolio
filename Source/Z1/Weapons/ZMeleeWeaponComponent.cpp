// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ZMeleeWeaponComponent.h"
#include "Z1Character.h"
#include "Animations/ZAnimInstance.h"
#include "Components/BoxComponent.h"
#include "Weapons/ZMeleeWeaponBase.h"
#include "Weapons/ZBaseWeapon.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/ZComboActionData.h"
#include "TimerManager.h"

// Sets default values for this component's properties
UZMeleeWeaponComponent::UZMeleeWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MeleeWeaponSocketName = FName(TEXT("hand_rSocket"));
}

// Called when the game starts
void UZMeleeWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	SpawnWeapon();

	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->GetMesh()->OnComponentHit.AddDynamic(this, &UZMeleeWeaponComponent::ComponentHit);
	}

}

void UZMeleeWeaponComponent::ComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 패링 로직 또는 데미지 처리 구현
	// 예: OtherActor가 적인지 확인하고 데미지 적용

}

// Called every frame
void UZMeleeWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UZMeleeWeaponComponent::SetCurrentWeaponFromInventory(TObjectPtr<AZMeleeWeaponBase> Weapon)
{
	CurrentWeapon = Weapon;
}

void UZMeleeWeaponComponent::SpawnWeapon()
{
	if (!IsValid(GetWorld()))
	{
		return;
	}

	TObjectPtr<ACharacter> Character = Cast<ACharacter>(GetOwner());
	if (IsValid(Character))
	{
		CurrentWeapon = GetWorld()->SpawnActor<AZMeleeWeaponBase>();
		if (IsValid(CurrentWeapon))
		{
			FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, false);
			CurrentWeapon->AttachToComponent(Character->GetMesh(), AttachmentRules, MeleeWeaponSocketName);
		}
	}
}

UZAnimInstance* UZMeleeWeaponComponent::GetCharacterAnimInstance() const
{
	if (AZ1Character* Character = Cast<AZ1Character>(GetOwner()))
	{
		if (USkeletalMeshComponent* MeshComp = Character->GetMesh())
		{
			if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
			{
				if (UZAnimInstance* CharacterAnimInstance = Cast<UZAnimInstance>(AnimInstance))
				{
					return CharacterAnimInstance;
				}
			}
		}
	}
	return nullptr;
}
