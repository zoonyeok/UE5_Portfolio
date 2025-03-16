// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZEquipmentManagerComponent.h"
#include "Items/ZInventoryItem.h"
#include "Items/ZWorldItem.h"
#include "Z1Character.h"
#include "ZGameTypes.h"
#include "Weapons/ZBaseWeapon.h"
#include <Animations/ZAnimInstance.h>

DEFINE_LOG_CATEGORY(LogEquipmentManager);


// Sets default values for this component's properties
UZEquipmentManagerComponent::UZEquipmentManagerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

bool UZEquipmentManagerComponent::EquipItem(EItemSlotType SlotType, const TObjectPtr<UZInventoryItem>& Item)
{
	if (!IsValid(Item))
	{
		return false;
	}

	// 캐릭터에서 착용가능한 아이템인지
	/* if (CharacterAttributeComp)
	{
		CharacterAttributeComp->UpdateStatsBasedOnItem(SlotType, Item);
	}*/

	// 슬롯 업데이트
	if (EquippedInventoryItems.Contains(SlotType))
	{
		return false;
	}

	// 캐릭터 능력치 업데이트
	/* if (CharacterAttributeComp)
	{
		CharacterAttributeComp->UpdateStatsBasedOnItem(SlotType, Item);
	}*/

	// 월드에 아이템 스폰
	TObjectPtr<AZWorldItem> WorldItem = EquipItemBySlotType(SlotType, Item);
	
	if (SlotType == EItemSlotType::EIS_Weapon_1)
	{
		if (TObjectPtr<AZBaseWeapon> Weapon = Cast<AZBaseWeapon>(WorldItem))
		{
			if (TObjectPtr<ACharacter> OwnerCharacter = Cast<ACharacter>(GetOwner()))
			{
				if (TObjectPtr<UZAnimInstance> AnimInstance = Cast<UZAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
				{
					AnimInstance->SetAnimationLayerByWeapon(Weapon->GetArmedCharacterAnimLayer());
					AnimInstance->PlayAnimMontage(Weapon->GetEquipAnimMontage());

					EquippedInventoryItems.Add(SlotType, Item);
					EquippedWorldItems.Add(SlotType, WorldItem);
	
					// 장착 이벤트 브로드캐스트
					OnEquippedItems.Broadcast(SlotType, Item);

					return true;
				}
			}
		}
	}

	if (IsValid(WorldItem))
	{
		return true;
	}
	
	return false;
}

bool UZEquipmentManagerComponent::UnEquipItem(EItemSlotType SlotType, const TObjectPtr<UZInventoryItem>& Item)
{
	if (!IsValid(Item))
	{
		return false;
	}

	// 슬롯 업데이트
	if (!EquippedInventoryItems.Contains(SlotType))
	{
		//LOG
		return false;
	}

	// 캐릭터 능력치 업데이트
	/* if (CharacterAttributeComp)
	{
		CharacterAttributeComp->UpdateStatsBasedOnItem(SlotType, Item);
	}*/
	
	EquippedInventoryItems.Remove(SlotType);
	
	/*if (OnEquipmentChanged.IsBound())
	{
		OnEquipmentChanged.Broadcast(SlotType, Item);
	}*/
	
	if (TObjectPtr<AZWorldItem> WorldItem = EquippedWorldItems.FindRef(SlotType))
	{
		if (TObjectPtr<AZBaseWeapon> Weapon = Cast<AZBaseWeapon>(WorldItem))
		{
			if (TObjectPtr<ACharacter> OwnerCharacter = Cast<ACharacter>(GetOwner()))
			{
				if (TObjectPtr<UZAnimInstance> AnimInstance = Cast<UZAnimInstance>(OwnerCharacter->GetMesh()->GetAnimInstance()))
				{
					if (IsValid(Weapon->GetDisarmedCharacterAnimLayer()) && IsValid(Weapon->GetDisarmAnimMontage()))
					{
						AnimInstance->SetAnimationLayerByWeapon(Weapon->GetDisarmedCharacterAnimLayer());
						AnimInstance->PlayAnimMontage(Weapon->GetDisarmAnimMontage());
						//TODO : 더 정확하게 맞추려면 Animnotify
						AnimInstance->OnMontageEnded.AddDynamic(this, &ThisClass::HandleDisarmMontageEnd);
						PendingWorldItem = Weapon;
					}

					//TODO : GetDisarmAnimMontage가 없을시 바로 아이템 삭제
					if (!IsValid(Weapon->GetDisarmAnimMontage()))
					{
						// 대기 중인 무기가 없으면 반환
						if (IsValid(PendingWorldItem))
						{
							// 대기 중인 월드 아이템을 삭제
							//TODO : Remove Dynamic
							PendingWorldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
							PendingWorldItem->Destroy();
							PendingWorldItem = nullptr;
						}
					}
				}
			}
		}
	}
	
	EquippedWorldItems.Remove(SlotType);

	return true;
}

void UZEquipmentManagerComponent::HandleDisarmMontageEnd(UAnimMontage* Montage, bool bInterrupted)
{
	// 대기 중인 무기가 없으면 반환
	if (!IsValid(PendingWorldItem))
	{
		return;
	}

	// 대기 중인 월드 아이템을 삭제
	PendingWorldItem->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	PendingWorldItem->Destroy();
	PendingWorldItem = nullptr;

	// 델리게이트 해제
	if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh())
		{
			if (UAnimInstance* AnimInstance = CharacterMesh->GetAnimInstance())
			{
				AnimInstance->OnMontageEnded.RemoveDynamic(this, &ThisClass::HandleDisarmMontageEnd);
			}
		}
	}
}


TObjectPtr<UZInventoryItem> UZEquipmentManagerComponent::GetEquippedItem(EItemSlotType SlotType) const
{
	if (const TObjectPtr<UZInventoryItem>* FoundItem = EquippedInventoryItems.Find(SlotType))
	{
		return *FoundItem;
	}

	// 장착된 아이템이 없으면 nullptr 반환
	return nullptr;
}

TObjectPtr<AZWorldItem> UZEquipmentManagerComponent::EquipItemBySlotType(EItemSlotType SlotType, UZInventoryItem* Item)
{
	// 아이템 데이터 유효성 검사
	if (!IsValid(Item) || !IsValid(GetOwner()))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("EquipItemBySlotType: Invalid Item or Owner."));
		return nullptr;
	}

	// 아이템 스태틱 데이터 가져오기
	FItemStaticData* ItemData = Item->GetStaticData();
	if (!ItemData)
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("EquipItemBySlotType: ItemStaticData is invalid."));
		return nullptr;
	}

	// 월드 아이템 스폰
	TObjectPtr<AZWorldItem> SpawnedWorldItem = GetWorld()->SpawnActor<AZWorldItem>(ItemData->WorldItemClass);
	if (!IsValid(SpawnedWorldItem))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("EquipItemBySlotType: Failed to spawn world item."));
		return nullptr;
	}

	if (SlotType == EItemSlotType::EIS_HealthPotion)
	{
		return SpawnedWorldItem;
	}

	// 캐릭터 및 메시 유효성 검사
	AZ1Character* OwnerCharacter = Cast<AZ1Character>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("EquipItemBySlotType: OwnerCharacter is invalid."));
		return nullptr;
	}
	
	USkeletalMeshComponent* CharacterMesh = OwnerCharacter->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("EquipItemBySlotType: CharacterMesh is invalid."));
		return nullptr;
	}

	// 소켓 유효성 검사 및 아이템 부착
	if (!CharacterMesh->DoesSocketExist(ItemData->ItemSocketName))
	{
		UE_LOG(LogEquipmentManager, Warning, TEXT("EquipItemBySlotType: Socket '%s' does not exist on the character mesh."),
			*ItemData->ItemSocketName.ToString());
		return nullptr;
	}

	SpawnedWorldItem->SetCollisionInSlot();
	SpawnedWorldItem->AttachToComponent(CharacterMesh, FAttachmentTransformRules::SnapToTargetIncludingScale, ItemData->ItemSocketName);
	UE_LOG(LogEquipmentManager, Log, TEXT("EquipItemBySlotType: Item successfully equipped to socket '%s'."), *ItemData->ItemSocketName.ToString());

	return SpawnedWorldItem;
}
