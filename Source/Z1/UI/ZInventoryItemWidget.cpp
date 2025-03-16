// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/ZInventoryItemWidget.h"
#include "UI/ZSpacialInventoryWidget.h"
#include "UI/ZInventoryGridWidget.h"
#include "Interfaces/ZItemDropWidgetInterface.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Items/ZInventoryItem.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "ZGameTypes.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "GameFrameWork/ZGameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"
#include <Blueprint/WidgetTree.h>
#include "ZEquipmentSlotWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogZInventoryWidget, All, All)

UZInventoryItemWidget::UZInventoryItemWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsDragging = false;
	bToggleDragging = false;
	bIsMousePressed = false;
	DragThreshold = 10.f;		   // 일반 드래그용 픽셀 거리 기준
	DragThresholdTime = 0.2f;      // 길게 누름 판별 시간(초)
}

void UZInventoryItemWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	UE_LOG(LogZInventoryWidget, Log, TEXT("NativeOnInitialized Called!"));
	CurrentColor = EquipableColor;
	// 드랍을 처리하기 위한 가시성 설정
	SetVisibility(ESlateVisibility::Visible);
	SetIsEnabled(true);
}

void UZInventoryItemWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogZInventoryWidget, Log, TEXT("NativeConstruct Called!"));
	Refresh();
}

void UZInventoryItemWidget::InitializeItemWidget(const TObjectPtr<UZInventoryGridWidget>& InGridWidget,
	const TObjectPtr<UZInventoryItem>& Item)
{
	GridWidget = InGridWidget;
	InventoryItem = Item;
	TileSize = GridWidget->GetTileSize();
	InventoryComponent = GridWidget->GetInventoryComponent();

	// 아이템 제거 시점에 함께 처리하기 위한 바인딩
	OnItemRemoved.AddDynamic(InventoryComponent, &UZSpatialInventoryComponent::RemoveItemByPointer);
}

void UZInventoryItemWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// 1) 마우스를 누른 상태이며, 아직 토글 드래그/일반 드래그 중이 아닐 때
	//    누르고 있던 시간이 DragThresholdTime을 초과하면 일반 드래그 시작
	if (bIsMousePressed && !bToggleDragging && !bIsDragging)
	{
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			FVector2D CurrentMousePosition;
			if (PlayerController->GetMousePosition(CurrentMousePosition.X, CurrentMousePosition.Y))
			{
				float ReleasedTime = GetWorld()->GetTimeSeconds();
				float HeldTime = ReleasedTime - PressedTime;

				if (HeldTime > DragThresholdTime)
				{
					StartDragging(CurrentMousePosition);
					UpdateDragPosition(CurrentMousePosition);
					//UpdateColor
				}
			}
		}
	}

	// 2) 일반 드래그 중이거나 토글 드래그 중이라면 마우스 위치를 추적해서 업데이트
	if (bIsDragging || bToggleDragging)
	{
		if (APlayerController* PlayerController = GetOwningPlayer())
		{
			FVector2D CurrentMousePosition;
			if (PlayerController->GetMousePosition(CurrentMousePosition.X, CurrentMousePosition.Y))
			{
				UpdateDragPosition(CurrentMousePosition);
				//UpdateColor
			}
		}
	}
}

FReply UZInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		bIsMousePressed = true;
		PressedTime = GetWorld()->GetTimeSeconds();
		InitialMousePosition = InMouseEvent.GetScreenSpacePosition();

		// "누른 시점"에 바로 토글 드래그를 하지 않고,
		// 짧은 클릭인지(토글), 길게 누른 건지(일반 드래그) Tick 또는 MouseUp에서 분기
		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UZInventoryItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsMousePressed = false;

		const float HeldTime = GetWorld()->GetTimeSeconds() - PressedTime;

		// 이미 일반 드래그 중이었다면 버튼 업과 함께 해제
		if (bIsDragging)
		{
			if (StopDragging(InMouseEvent) && bToggleDragging)
			{
				bToggleDragging = false;
			}
		}
		else
		{
			// (1) 마우스 업 시점에 "짧은 클릭"이었다면 → 토글 드래그를 토글
			// (2) 이미 "길게 누름"으로 넘어가면 Tick에서 StartDragging이 실행
			//     (즉 HeldTime이 DragThresholdTime보다 큰 경우, OnMouseButtonUp에서는 따로 처리 없음)
			bToggleDragging = !bToggleDragging;

			// "짧게 누름"일 때만 새로 토글 드래그 시작
			if (HeldTime < DragThresholdTime && bToggleDragging)
			{
				StartDragging(InMouseEvent.GetScreenSpacePosition());
			}
		}

		return FReply::Handled();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UZInventoryItemWidget::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseEnter(InGeometry, InMouseEvent);
	BackgroundBorder->SetBrushColor(HoveringItemColor);
}

void UZInventoryItemWidget::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	Super::NativeOnMouseLeave(InMouseEvent);
	BackgroundBorder->SetBrushColor(CurrentColor);
}

void UZInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);

	UDragDropOperation* DragOperation = NewObject<UDragDropOperation>();
	DragOperation->DefaultDragVisual = this;
	//DragOperation->DefaultDragVisual->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	DragOperation->Payload = this;
	DragOperation->Pivot = EDragPivot::MouseDown;
	OutOperation = DragOperation;
}

FReply UZInventoryItemWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return FReply::Unhandled(); 
	//return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

bool UZInventoryItemWidget::NativeOnDragOver(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	//TODO : 드래그되는 동안 들어갈 인벤토리 자리에 색깔칠해주기 (아이템 중심과 그 위치의 타일 4등분해서 기준잡기)
	// 드래그되는 동안 다른 아이템을 발견
	return false;
}

void UZInventoryItemWidget::Refresh()
{
	if (!IsValid(InventoryItem))
	{
		//LOG
		UE_LOG(LogZInventoryWidget, Log, TEXT("InventoryItem is InValid!"));
		return;
	}

	ItemDisplaySize = InventoryItem.Get()->GetGridSize() * TileSize;

	BackgroundSizeBox->SetWidthOverride(ItemDisplaySize.X);
	BackgroundSizeBox->SetHeightOverride(ItemDisplaySize.Y);

	UCanvasPanelSlot* ItemCanvas = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemImage);
	ItemCanvas->SetSize(ItemDisplaySize);
}

bool UZInventoryItemWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UZInventoryItemWidget::SetParentWidget(TWeakObjectPtr<UUserWidget> NewParentWidget)
{
	if (NewParentWidget.IsValid())
	{
		ParentWidget = NewParentWidget;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ParentWidget assigned!"));
	}
}

FSlateBrush UZInventoryItemWidget::GetIconImage() const
{
	if (IsValid(InventoryItem))
	{
		UMaterialInstanceDynamic* ItemIcon = InventoryItem->GetIconMaterial(false);
		return UWidgetBlueprintLibrary::MakeBrushFromMaterial(ItemIcon, ItemDisplaySize.X, ItemDisplaySize.Y);
	}
	
	UE_LOG(LogZInventoryWidget, Log, TEXT("ItemIcon is InValid!"));
	return FSlateBrush();
}

void UZInventoryItemWidget::SetItemBackgroundColor()
{
	//Character능력치 확인 후 SetColor
}

void UZInventoryItemWidget::StartDragging(const FVector2D& MousePosition)
{
	bIsDragging = true;

	//TODO : Delegate?
	if (ParentWidget.IsValid())
	{
		if (UZEquipmentSlotWidget* SlotWidget = Cast<UZEquipmentSlotWidget>(ParentWidget))
		{
			if (UZEquipmentManagerComponent* EquipComp = SlotWidget->GetEquipComponent())
			{
				EquipComp->UnEquipItem(SlotWidget->GetItemSlotType(), SlotWidget->GetItem());
			}
		}
	}

	this->RemoveFromParent();

	FVector2D WidgetSize = this->GetDesiredSize();
	this->SetDesiredSizeInViewport(WidgetSize);

	FVector2D AdjustedPosition = MousePosition - (WidgetSize * 0.5f);
	this->SetPositionInViewport(AdjustedPosition, true);
	this->AddToViewport();

	GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Green, TEXT("Dragging Started"));
}

bool UZInventoryItemWidget::StopDragging(const FPointerEvent& InMouseEvent)
{
	FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
	TObjectPtr<UZSpacialInventoryWidget> SpacialInventoryWidget = GridWidget->GetSpacialInventoryWidget();

	auto DropTargets = SpacialInventoryWidget->GetDropTargets();
	for (auto& DropTarget : DropTargets)
	{
		if (DropTarget && DropTarget->Implements<UZItemDropWidgetInterface>())
		{
			if (IZItemDropWidgetInterface* DropTargetInterface = Cast<IZItemDropWidgetInterface>(DropTarget))
			{
				if (DropTargetInterface->HandleDropItem(this, MousePosition))
				{
					bIsDragging = false;
					GEngine->AddOnScreenDebugMessage(-1, 1.5f, FColor::Red, TEXT("Dragging Stopped"));
					return true; // 첫 번째 유효한 대상에 대해 처리 후 종료
				}
			}
		}
	}

	return false;
}

void UZInventoryItemWidget::UpdateDragPosition(const FVector2D& MousePosition)
{
	FVector2D WidgetSize = this->GetDesiredSize();
	FVector2D AdjustedPosition = MousePosition - (WidgetSize * 0.5f);
	SetPositionInViewport(AdjustedPosition, true);

	//여기서 예상 타일에 색칠?
	FVector2D ItemGridSize = InventoryItem->GetGridSize();
	GridWidget->SetTileColorInGrid(MousePosition, ItemGridSize, FColor::Green);
}
