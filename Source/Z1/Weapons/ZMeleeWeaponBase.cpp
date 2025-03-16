// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ZMeleeWeaponBase.h"

#include "EditorCategoryUtils.h"
#include "Z1Character.h"
#include "Components/ZAttributeComponent.h"
#include "Components/ZWeaponTraceComponent.h"

AZMeleeWeaponBase::AZMeleeWeaponBase()
{
	/*static ConstructorHelpers::FObjectFinder<USkeletalMesh> SK_WEAPON(TEXT("/Game/KatanaAnimationSet/Meshes/SK_Katana.SK_Katana"));
	if (SK_WEAPON.Succeeded())
	{
		SkeletalMeshComponent->SetSkeletalMesh(SK_WEAPON.Object);
	}

	ItemCategory = EItemCategory::MeleeWeapon;*/
	//WeaponSocketName = ""
	//ItemCategory = "Katana";

	TraceComponent = CreateDefaultSubobject<UZWeaponTraceComponent>("WeaponTraceComp");
}

void AZMeleeWeaponBase::BeginPlay()
{
    Super::BeginPlay();

	//TODO
	//InitializeItem()
}

void AZMeleeWeaponBase::StartAttack()
{
    Super::StartAttack();
}

void AZMeleeWeaponBase::EndAttack()
{
    Super::EndAttack();
}

EInventoryActionResult AZMeleeWeaponBase::PickUp(APawn* Player)
{
	return Super::PickUp(Player);
	//return EInventoryActionResult();
}

UZInventoryItem* AZMeleeWeaponBase::ConvertToInventoryItem()
{
	return Super::ConvertToInventoryItem();
	//return nullptr;
}

void AZMeleeWeaponBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
		
	//SkeletalMeshComponent->OnComponentBeginOverlap.AddDynamic(this, &AZMeleeWeaponBase::OnWeaponOverlap);
	SkeletalMeshComponent->OnComponentHit.AddDynamic(this, &AZMeleeWeaponBase::OnWeaponHit);
}

void AZMeleeWeaponBase::OnWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}
	
	if (TObjectPtr<APawn> Pawn = Cast<APawn>(OtherActor))
	{
		if (TObjectPtr<UZAttributeComponent> AttributeComp = Pawn->FindComponentByClass<UZAttributeComponent>())
		{
			float DamageAmount = CalculateFinalDamage();  // ✅ 기본 데미지 값 설정
			AttributeComp->ChangeCurrentHP(GetOwner(), -DamageAmount);
			UE_LOG(LogTemp, Log, TEXT("Weapon hit %s, applied %f damage"), *Pawn->GetName(), DamageAmount);
		}
	}
}

void AZMeleeWeaponBase::OnWeaponHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!IsValid(OtherActor) || OtherActor == GetOwner())
	{
		return;
	}

	if (TObjectPtr<APawn> Pawn = Cast<APawn>(OtherActor))
	{
		if (TObjectPtr<UZAttributeComponent> AttributeComp = Pawn->FindComponentByClass<UZAttributeComponent>())
		{
			float DamageAmount = CalculateFinalDamage();  // ✅ 기본 데미지 값 설정
			AttributeComp->ChangeCurrentHP(GetOwner(), -DamageAmount);
			UE_LOG(LogTemp, Log, TEXT("Weapon hit %s, applied %f damage"), *Pawn->GetName(), DamageAmount);
		}
	}
}

float AZMeleeWeaponBase::CalculateFinalDamage()
{
	float DamageAmount = 20.0f;

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (IsValid(Character))
	{
		UZAttributeComponent* AttributeComp = Character->FindComponentByClass<UZAttributeComponent>();
		if (IsValid(AttributeComp))
		{
			FCharacterStats Stats = AttributeComp->GetFCharacterStats();
			FinalDamage = (DamageAmount + AttackDamage) * (1.0f + Stats.Strength * 0.05f) + (Stats.Dexterity * 0.02f);
			return FinalDamage;
		}
	}
	
	return DamageAmount;
}
