// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZWorldItemWidget.generated.h"

class UWorldItemWidgetComponent;
class UTextBlock;
/**
 * 
 */
UCLASS()
class Z1_API UZWorldItemWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UZWorldItemWidget(const FObjectInitializer& ObjectInitializer);

	/*void SetItemNameText(const FText& Text);*/

public:
	UFUNCTION(BlueprintCallable, Category = "Interaction")
	void UpdateInteractionWidget(UWorldItemWidgetComponent* InteractionItemComponent);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInteractionwidget();

	UPROPERTY(BlueprintReadOnly, Category = "Interaction", meta = (ExposeOnSpawn))
	UWorldItemWidgetComponent* OwningInteractionComponent;

//protected:
//	UPROPERTY(meta = (BindWidget))
//	UTextBlock* ItemNameText;
};
