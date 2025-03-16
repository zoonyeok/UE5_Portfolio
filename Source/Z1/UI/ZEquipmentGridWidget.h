// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/ZItemDropWidgetInterface.h"
#include "ZGameTypes.h"
#include "ZEquipmentGridWidget.generated.h"

class UBorder;
class UCanvasPanel;
class UZEquipmentManagerComponent;
class UZEquipmentSlotWidget;

/**
 * 
 */
UCLASS()
class Z1_API UZEquipmentGridWidget : public UUserWidget, public IZItemDropWidgetInterface
{
	GENERATED_BODY()
public:

	void InitializeEquipWidget(const TObjectPtr<UZEquipmentManagerComponent>& EquipmentComponent);
	virtual bool HandleDropItem(class UZInventoryItemWidget* DroppedWidget, const FVector2D& MousePosition) override;

protected:
	virtual void NativeOnInitialized() override;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory", meta = (ExposeOnSpawn = "true"))
	TObjectPtr<UZEquipmentManagerComponent> EquipmentComponent;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> HelmetSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> ArmorSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> BeltSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> GloveSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> BootSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> WeaponSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> WeaponSlot_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> RingSlot_1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> RingSlot_2;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> NecklaceSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> HealthPotionSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UZEquipmentSlotWidget> ManaPotionSlot;

	UPROPERTY()
	TMap<EItemSlotType, TObjectPtr<UZEquipmentSlotWidget>> EquipmentSlotMap;
};
