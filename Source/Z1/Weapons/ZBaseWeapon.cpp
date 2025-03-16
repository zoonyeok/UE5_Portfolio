// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ZBaseWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ZInventoryComponent.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "Z1Character.h"
#include "ZGameTypes.h"
#include "Components/ZAttributeComponent.h"

#include "Items/ZWeaponInventoryItem.h"
#include "UI/WorldItemWidgetComponent.h"

// Sets default values
AZBaseWeapon::AZBaseWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	RootComponent = SkeletalMeshComponent;

	// 충돌 설정
    SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    //ECC_GameTraceChannel1, WorldItem
    //ECC_GameTraceChannel2, InventoryItem
    SkeletalMeshComponent->SetCollisionObjectType(ECollisionChannel::ECC_GameTraceChannel1);
    SkeletalMeshComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    //SkeletalMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    SkeletalMeshComponent->SetCollisionResponseToChannel(ECC_Pawn, ECollisionResponse::ECR_Overlap);
    WorldItemWidgetComponent->SetupAttachment(RootComponent);

    SkeletalMeshComponent->IgnoreActorWhenMoving(GetOwner(), true);
}

// Called when the game starts or when spawned
void AZBaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (!LoadMesh())
	{
		//LOG
	}
}

void AZBaseWeapon::StartAttack()
{
	
}

void AZBaseWeapon::EndAttack()
{
}

float AZBaseWeapon::CalculateFinalDamage()
{
	float BaseDamage = 20.0f;

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (IsValid(Character))
	{
		UZAttributeComponent* AttributeComp = Character->FindComponentByClass<UZAttributeComponent>();
		if (IsValid(AttributeComp))
		{
			FCharacterStats Stats = AttributeComp->GetFCharacterStats();
            FinalDamage = (BaseDamage + AttackDamage) * (1.0f + Stats.Strength * 0.05f) + (Stats.Dexterity * 0.02f);
			return FinalDamage;
		}
	}
	
	return BaseDamage;
}

EInventoryActionResult AZBaseWeapon::PickUp_Implementation(APawn* Player)
{
    AZ1Character* Character = Cast<AZ1Character>(Player);
    if (!IsValid(Character))
    {
        // LOG
        return EInventoryActionResult::Fail;
    }

    //
    UZSpatialInventoryComponent* InvenComp = Character->GetComponentByClass<UZSpatialInventoryComponent>();
    if (!IsValid(InvenComp))
    {
        // LOG
        return EInventoryActionResult::Fail;
    }

    UZWeaponInventoryItem* WeaponInvenItem = Cast<UZWeaponInventoryItem>(ConvertToInventoryItem());
    if (WeaponInvenItem)
    {
        bool ItemAddResult = false;
        ItemAddResult = InvenComp->TryAddItem(WeaponInvenItem);
        if (ItemAddResult)
        {
            //TODO : Widget Update, with Delegate
            //World에서 지우거나, object pool에 넣기
            //Inventory Slot이 비었다면 삭제X, 맞는 슬롯에 추가
            this->Destroy();
            return EInventoryActionResult::Success;
        }
    }

    return EInventoryActionResult::Fail;
}

UZInventoryItem* AZBaseWeapon::ConvertToInventoryItem()
{
    UZWeaponInventoryItem* WeaponInvenItem = NewObject<UZWeaponInventoryItem>();
    WeaponInvenItem->Initialize(ItemDataTable, ItemID, 1);

    if (WeaponInvenItem)
    {
        return WeaponInvenItem;
    }

    return nullptr;
}


