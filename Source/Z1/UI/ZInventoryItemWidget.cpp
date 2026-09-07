// Fill out your copyright notice in the Description page of Project Settings.
#include "UI/ZInventoryItemWidget.h"
#include "Framework/Application/SlateApplication.h"
#include "UI/ZSpacialInventoryWidget.h"
#include "UI/ZInventoryGridWidget.h"
#include "Interfaces/ZItemDropWidgetInterface.h"
#include "Components/CanvasPanelSlot.h"
#include "Items/ZInventoryItem.h"
#include "Components/SizeBox.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Blueprint/DragDropOperation.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "GameFramework/PlayerController.h"
#include "ZEquipmentSlotWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogZInventoryWidget, All, All)

UZInventoryItemWidget::UZInventoryItemWidget(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	bIsDragging = false;
	bToggleDragging = false;
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
	if (bIsDragging)
	{
		UpdateDragPosition(FSlateApplication::Get().GetCursorPos());
	}
}

FReply UZInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		UZSpacialInventoryWidget* Inventory = IsValid(GridWidget) ? GridWidget->GetSpacialInventoryWidget().Get() : nullptr;
		if (Inventory && IsValid(Inventory->DraggedItemWidget))
		{
			Inventory->DraggedItemWidget->StopDragging(InMouseEvent);
		}
		else
		{
			StartDragging(InMouseEvent.GetScreenSpacePosition());
		}
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UZInventoryItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

FReply UZInventoryItemWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	return NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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
	// Click-to-carry uses its own cursor widget, not a Slate drag/drop operation.
	OutOperation = nullptr;
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
	if (!IsValid(GridWidget) || !IsValid(InventoryItem))
	{
		return;
	}
	UZSpacialInventoryWidget* Inventory = GridWidget->GetSpacialInventoryWidget();
	if (!IsValid(Inventory))
	{
		return;
	}
	// Keep the cursor widget alive even when RefreshGrid rebuilds its old panel.
	Inventory->DraggedItemWidget = this;
	bIsDragging = true;
	bToggleDragging = false;
	if (UZEquipmentSlotWidget* SlotWidget = Cast<UZEquipmentSlotWidget>(ParentWidget.Get()))
	{
		if (UZEquipmentManagerComponent* EquipComp = SlotWidget->GetEquipComponent())
		{
			EquipComp->UnEquipItem(SlotWidget->GetItemSlotType(), SlotWidget->GetItem());
		}
	}
	ParentWidget.Reset();
	ItemDisplaySize = InventoryItem->GetGridSize() * TileSize;
	RemoveFromParent();
	SetDesiredSizeInViewport(ItemDisplaySize);
	SetAlignmentInViewport(FVector2D(0.5f, 0.5f));
	AddToViewport(100);
	UpdateDragPosition(MousePosition);
}

void UZInventoryItemWidget::CancelDragging()
{
	bIsDragging = false;
	bToggleDragging = false;
	if (IsInViewport() || GetParent())
	{
		RemoveFromParent();
	}
}

bool UZInventoryItemWidget::StopDragging(const FPointerEvent& InMouseEvent)
{
	FVector2D MousePosition = InMouseEvent.GetScreenSpacePosition();
	TObjectPtr<UZSpacialInventoryWidget> SpacialInventoryWidget = GridWidget->GetSpacialInventoryWidget();

	if (!IsValid(SpacialInventoryWidget))
	{
		return false;
	}
	if (!SpacialInventoryWidget->IsOverInventoryPanels(MousePosition))
	{
		return SpacialInventoryWidget->HandleDropItem(this, MousePosition);
	}

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
					if (SpacialInventoryWidget->DraggedItemWidget == this)
					{
						SpacialInventoryWidget->DraggedItemWidget = nullptr;
					}
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
	// MousePosition is always an absolute Slate coordinate, including in PIE.
	const FVector2D ViewportPosition =
		UWidgetLayoutLibrary::GetViewportWidgetGeometry(this).AbsoluteToLocal(MousePosition);
	SetPositionInViewport(ViewportPosition, false);
	GridWidget->SetTileColorInGrid(MousePosition, InventoryItem->GetGridSize(), FColor::Green);
}
