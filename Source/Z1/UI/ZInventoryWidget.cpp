// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZInventoryWidget.h"
#include "Components/TextBlock.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "Items/ZInventoryItem.h"

UZInventoryWidget::UZInventoryWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	SetIsFocusable(true);
}

void UZInventoryWidget::SetMoney(const FText& Text)
{
	if (IsValid(MoneyTextBlock))
	{
		MoneyTextBlock->SetText(Text);
	}
}

void UZInventoryWidget::SetTotalWeight(const FText& Text)
{
	if (IsValid(TotalWeightTextBlock))
	{
		TotalWeightTextBlock->SetText(Text);
	}
}

void UZInventoryWidget::SetCurrentWeight(const FText& Text)
{
	if (IsValid(CurrentWeightTextBlock))
	{
		CurrentWeightTextBlock->SetText(Text);
	}
}

void UZInventoryWidget::SetLevelNubmer(const FText& Text)
{
	if (IsValid(LevelNumberTextBlock))
	{
		LevelNumberTextBlock->SetText(Text);
	}
}

bool UZInventoryWidget::Initialize()
{
	return Super::Initialize();
}

void UZInventoryWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InPointerEvent, UDragDropOperation*& OutOperation)
{
	// 드래그 드롭 오퍼레이션 생성
	UDragDropOperation* DragDropOp = UWidgetBlueprintLibrary::CreateDragDropOperation(UDragDropOperation::StaticClass());

	// 드래그 시 보여줄 위젯 설정
	DragDropOp->DefaultDragVisual = this;
	DragDropOp->Payload = InventoryItem; // 드래그 시 전달할 데이터

	OutOperation = DragDropOp;

	// 현재 슬롯에서 아이템 제거 로직 필요
}

bool UZInventoryWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	if (UDragDropOperation* DragDropOp = Cast<UDragDropOperation>(InOperation))
	{
		// Payload를 UZInventoryItem으로 캐스팅
		UZInventoryItem* DroppedItem = Cast<UZInventoryItem>(DragDropOp->Payload);
		if (DroppedItem)
		{
			// 드롭된 아이템 처리 로직
			// TODO : InventoryItem에서 처리할지, SpawnManager를 따로 만들지
			//ProcessDroppedItem(DroppedItem);
			if (DroppedItem->SpawnWorldItem())
			{
				return true;// 드롭 성공
			}
		}
	}
	return false; // 드롭 실패
}
