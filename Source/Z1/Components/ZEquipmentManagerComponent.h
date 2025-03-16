// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZGameTypes.h"
#include "ZEquipmentManagerComponent.generated.h"

class UZInventoryItem;
class UZAttributeComponent;
class AZWorldItem;
class AZBaseWeapon;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipItem, EItemSlotType, SlotType, UZInventoryItem*, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterEquipItem, UZInventoryItem*, InventoryItem, AZWorldItem*, WorldItem);

DECLARE_LOG_CATEGORY_EXTERN(LogEquipmentManager, Log, All);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class Z1_API UZEquipmentManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZEquipmentManagerComponent();

	// 캐릭터 능력치 업데이트 등 처리
	bool EquipItem(EItemSlotType SlotType, const TObjectPtr<UZInventoryItem>& Item);
	bool UnEquipItem(EItemSlotType SlotType, const TObjectPtr<UZInventoryItem>& Item);
	
	// 현재 장착된 아이템 반환
	TObjectPtr<UZInventoryItem> GetEquippedItem(EItemSlotType SlotType) const;

	//Character Animation변경
	UPROPERTY(BlueprintAssignable, Category = "Equipment")
	FOnEquipItem OnEquippedItems;
	
	TObjectPtr<AZWorldItem> EquipItemBySlotType(EItemSlotType SlotType, UZInventoryItem* Item);
	
private:
	// 각 슬롯별 장착된 아이템 관리
	UPROPERTY()
	TMap<EItemSlotType, TObjectPtr<UZInventoryItem>> EquippedInventoryItems;

	UPROPERTY()
	TMap<EItemSlotType, TObjectPtr<AZWorldItem>> EquippedWorldItems;

	// 캐릭터 레퍼런스 (능력치 업데이트용)
	UPROPERTY()
	TObjectPtr<UZAttributeComponent> CharacterAttributeComp;

	// 삭제를 대기 중인 무기
	UPROPERTY()
	TObjectPtr<AZBaseWeapon> PendingWorldItem;
	
	UFUNCTION()
	void HandleDisarmMontageEnd(UAnimMontage* Montage, bool bInterrupted);
};

//TODO : 