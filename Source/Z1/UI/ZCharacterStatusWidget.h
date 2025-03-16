// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZCharacterStatusWidget.generated.h"

class UZAttributeComponent;
class UImage;
class UMaterialInstanceDynamic;
/**
 * 
 */
UCLASS()
class Z1_API UZCharacterStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void InitializeStatusWidget(const TObjectPtr<UZAttributeComponent>& AttributeComp);

protected:
	virtual void NativeOnInitialized() override;

	virtual void NativeConstruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HealthSphere;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> HealthDynamicMaterial;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ManaSphere;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ManaDynamicMaterial;

	UPROPERTY()
	TObjectPtr<UZAttributeComponent> AttributeComp;

	UFUNCTION()
	void HandleHpChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta);
	UFUNCTION()
	void HandleMpChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta);
	
	void CreateMaterialDynamic(TObjectPtr<UImage> AttributeImage, TObjectPtr<UMaterialInstanceDynamic>& AttributeMaterialDynamic);
};
