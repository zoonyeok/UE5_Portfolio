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
		StopAction(Instigator);
		return;
	}
	
	if (TObjectPtr<AZ1Character> Character = Cast<AZ1Character>(GetCharacter()))
	{
		if (TObjectPtr<UZEquipmentManagerComponent> EquipmentManagerComponent = Character->GetComponentByClass<UZEquipmentManagerComponent>())
		{
			if (TObjectPtr<UZInventoryItem> Item = EquipmentManagerComponent->GetEquippedItem(EItemSlotType::EIS_HealthPotion))
			{
				if (TObjectPtr<UZPotionInventoryItem> PotionInventoryItem = Cast<UZPotionInventoryItem>(Item))
				{
					PotionInventoryItem->Execute_UseItem(PotionInventoryItem, Character);

					AnimInstance->Montage_Play(ItemUseMontage);

					FOnMontageEnded EndDelegate;
					EndDelegate.BindUObject(this, &UZItemXButtonAction::OnItemUseMontageEnded);
					AnimInstance->Montage_SetEndDelegate(EndDelegate, ItemUseMontage);
				}
			}
		}
	}
}

void UZItemXButtonAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);
}

void UZItemXButtonAction::OnItemUseMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == ItemUseMontage && !bInterrupted)
	{
		StopAction(GetCharacter());
	}
}
