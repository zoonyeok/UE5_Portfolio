// Fill out your copyright notice in the Description page of Project Settings.


#include "ZItemXButtonAction.h"

#include "Z1Character.h"
#include "Animations/ZAnimInstance.h"
#include "Components/ZActionComponent.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "Items/ZPotionInventoryItem.h"

UZItemXButtonAction::UZItemXButtonAction()
{
}

void UZItemXButtonAction::StartAction(AActor* Instigator)
{
	Super::StartAction(Instigator);

	UZAnimInstance* AnimInstance = GetOwningAnimInstance();
	if (!IsValid(AnimInstance) || AnimInstance->Montage_IsPlaying(ItemUseMontage))
	{
		return;
	}
	
	TObjectPtr<AZ1Character> Character = Cast<AZ1Character>(GetCharacter());
	if (!Character) return;

	TObjectPtr<UZEquipmentManagerComponent> EquipmentManagerComponent = Character->GetComponentByClass<UZEquipmentManagerComponent>();
	if (!EquipmentManagerComponent) return;

	TObjectPtr<UZInventoryItem> Item = EquipmentManagerComponent->GetEquippedItem(EItemSlotType::EIS_HealthPotion);
	if (!Item) return;

	TObjectPtr<UZPotionInventoryItem> PotionInventoryItem = Cast<UZPotionInventoryItem>(Item);
	if (!PotionInventoryItem) return;

	PotionInventoryItem->UseItem(Character);
	
	AnimInstance->Montage_Play(ItemUseMontage);

	// Montage 종료 시점에 호출될 델리게이트 바인딩
	FOnMontageEnded EndDelegate;
	EndDelegate.BindUObject(this, &UZItemXButtonAction::OnItemUseMontageEnded);
	AnimInstance->Montage_SetEndDelegate(EndDelegate, ItemUseMontage);
}

void UZItemXButtonAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);
}

void UZItemXButtonAction::OnItemUseMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ItemUseMontage && !bInterrupted)
	{
		AActor* Instigator = GetOwningComponent()->GetOwner();
		if (IsValid(Instigator))
		{
			StopAction(Instigator);
		}
	}
}
