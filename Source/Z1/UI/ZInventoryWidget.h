// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZInventoryWidget.generated.h"

class UImage;
class UTextBlock;
class UZInventoryItem;
/**
 * 
 */
UCLASS()
class Z1_API UZInventoryWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

	UZInventoryWidget(const FObjectInitializer& ObjectInitializer);

	void SetMoney(const FText& Text);
	void SetTotalWeight(const FText& Text);
	void SetCurrentWeight(const FText& Text);
	void SetLevelNubmer(const FText& Text);

	virtual bool Initialize() override;

	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InPointerEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

protected:

	UPROPERTY(BlueprintReadWrite, Meta = (ExposeOnSpawn = true))
	TObjectPtr<UZInventoryItem> InventoryItem;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MoneyTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalWeightTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentWeightTextBlock;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LevelNumberTextBlock;
};
