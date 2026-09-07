// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ZInventoryGridWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "Layout/Geometry.h"
#include "Rendering/DrawElements.h"
#include "Math/Vector2D.h"
#include "Math/Color.h"
#include <Blueprint/WidgetBlueprintLibrary.h>
#include "ZSpacialInventoryWidget.h"
#include "Items/ZInventoryItem.h"
#include "UI/ZInventoryItemWidget.h"
#include "UI/ZEquipmentGridWidget.h"
//#include <Blueprint/WidgetLayoutLibrary.h>

DEFINE_LOG_CATEGORY(LogZGridWidget);

void UZInventoryGridWidget::InitializeWidget(const TObjectPtr<UZSpacialInventoryWidget>& InInventoryWidget)
{
	SpacialInventoryWidget = InInventoryWidget;
	this->InventoryComponent = InInventoryWidget.Get()->GetInventoryComponent();
	this->TileSize = InInventoryWidget.Get()->GetTileSize();

	if (IsValid(GridBorder) && IsValid(this->InventoryComponent))
	{
		// Border의 슬롯을 CanvasPanelSlot으로 캐스팅
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(GridBorder->Slot))
		{
			GridSize = InventoryComponent.Get()->GetGridSize();
			FVector2D NewSize = FVector2D(GridSize.X * TileSize, GridSize.Y * TileSize);
			CanvasSlot->SetSize(NewSize);
			CreateLineSegments();
			RefreshGrid();
			InventoryComponent->OnInventoryChanged.AddUniqueDynamic(this, &ThisClass::RefreshGrid);
		}
	}

	PrevItemDropPosition = FIntPoint::ZeroValue;
}

void UZInventoryGridWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

bool UZInventoryGridWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	return Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
}

void UZInventoryGridWidget::CreateLineSegments()
{
	if (!IsValid(InventoryComponent))
	{
		return;
	}

	int ColumnSize = InventoryComponent.Get()->GetGridSize().Y;
	int RowSize = InventoryComponent.Get()->GetGridSize().X;

	if (ColumnSize <= 0 || RowSize <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid grid size: ColumnSize=%d, RowSize=%d"), ColumnSize, RowSize);
		return;
	}

	GridLines.Reset();
	GridLines.Reserve(ColumnSize + RowSize);

	// Vertical Lines
	for (int i = 0; i <= RowSize; i++)
	{
		float X = i * TileSize;
		GridLines.Emplace(FLine(X, 0, X, ColumnSize * TileSize));
	}

	// Horizontal Lines
	for (int i = 0; i <= ColumnSize; i++)
	{
		float Y = i * TileSize;
		GridLines.Emplace(FLine(0, Y, RowSize * TileSize, Y));
	}
}

int32 UZInventoryGridWidget::NativePaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	LayerId = Super::NativePaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	if (!IsValid(GridBorder))
	{
		UE_LOG(LogZGridWidget, Warning, TEXT("GridBorder is not valid in NativePaint."));
		return LayerId;
	}

	FVector2D TopLeft = GridBorder->GetCachedGeometry().GetLocalPositionAtCoordinates(FVector2D(0, 0));
	TArray<FVector2D> LinePoints;
	for (const FLine& Line : GridLines)
	{
		FVector2D StartPoint = Line.Start + TopLeft;
		FVector2D EndPoint = Line.End + TopLeft;
		LinePoints.Add(StartPoint);
		LinePoints.Add(EndPoint);
	}

	// 선의 색상과 두께, 매개변수로?
	FLinearColor LineColor = FLinearColor(0.5f, 0.5f, 0.5f, 0.5f);
	float LineThickness = 2.0f;
	FPaintContext Context(AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);
	for (int32 i = 0; i < LinePoints.Num(); i += 2)
	{
		TArray<FVector2D> SingleLine;
		SingleLine.Add(LinePoints[i]);
		SingleLine.Add(LinePoints[i + 1]);

		UWidgetBlueprintLibrary::DrawLines(
			Context,
			SingleLine,
			LineColor,
			true,
			LineThickness
		);
	}

	// 단일 강조 박스만 렌더링
	if (bIsHighlighted)
	{
		FSlateDrawElement::MakeBox(
			OutDrawElements,
			LayerId,
			AllottedGeometry.ToPaintGeometry(HighlightedTileLocation + TopLeft, HighlightedTileSize),
			FCoreStyle::Get().GetBrush("WhiteBrush"),
			ESlateDrawEffect::None,
			HighlightedTileColor
		);
	}

	return LayerId + 1;
}

//TODO : 매번 새롭게 그리는게 좋은건지?
void UZInventoryGridWidget::RefreshGrid()
{
	GridCanvasPanel.Get()->ClearChildren();

	InventoryItemWidgetMap.Empty(); //?

	auto ItemMap = InventoryComponent->GetInventoryItems();
	for (auto& Item : ItemMap)
	{
		if (SpacialInventoryWidget->DraggedItemWidget &&
			SpacialInventoryWidget->DraggedItemWidget->GetInventoryItem() == Item.Key)
		{
			continue;
		}
		UZInventoryItemWidget* ItemWidget = CreateWidget<UZInventoryItemWidget>(GetOwningPlayer(), InventoryItemWidgetClass);
		if (IsValid(ItemWidget))
		{
			//SpacialInventoryWidget
			ItemWidget->InitializeItemWidget(this, Item.Key);

			if (UCanvasPanelSlot* ItemCanvasPanel = Cast<UCanvasPanelSlot>(GridCanvasPanel.Get()->AddChild(ItemWidget)))
			{
				ItemCanvasPanel->SetAutoSize(true);
				ItemCanvasPanel->SetPosition(Item.Value * TileSize);
			}

			InventoryItemWidgetMap.Add(Item.Value, ItemWidget);
		}
	}
}

bool UZInventoryGridWidget::IsItemDropHighlightOutOfBounds(const FIntPoint& ItemDropPosition, const FIntPoint& ItemSize)
{
	return (ItemDropPosition.X < 0 || ItemDropPosition.X + ItemSize.X > GridSize.X ||
		ItemDropPosition.Y < 0 || ItemDropPosition.Y + ItemSize.Y > GridSize.Y);
}

bool UZInventoryGridWidget::HandleSwap(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem, const FVector2D& InMousePosition, const FIntPoint& ItemDropPos)
{
	UZInventoryItem* ItemInGrid = InventoryComponent->GetItemAtPosition(DroppedItem, ItemDropPos, DroppedItem->GetGridSize());
	if (!IsValid(ItemInGrid))
	{
		return false;
	}
	const FIntPoint FoundItemPos = InventoryComponent->FindItemPosition(ItemInGrid);
	UZInventoryItemWidget* WidgetInGrid = InventoryItemWidgetMap.FindRef(FoundItemPos);
	if (!IsValid(WidgetInGrid))
	{
		return false;
	}
	// Detach the displaced widget before the synchronous grid refresh.
	WidgetInGrid->StartDragging(InMousePosition);
	DroppedWidget->RemoveFromParent();
	InventoryComponent->SwapItems(ItemInGrid, DroppedItem, ItemDropPos);
	bIsHighlighted = false;
	return true;
}

bool UZInventoryGridWidget::HandleAddItem(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem, const FIntPoint& ItemDropPos)
{
	const FIntPoint OriginalPosition = InventoryComponent->FindItemPosition(DroppedItem);
	if (OriginalPosition.X >= 0)
	{
		InventoryComponent->RemoveItemByGridPosition(OriginalPosition, false);
	}
	if (!InventoryComponent->AddItem(DroppedItem, ItemDropPos, false))
	{
		if (OriginalPosition.X >= 0)
		{
			InventoryComponent->AddItem(DroppedItem, OriginalPosition, false);
		}
		return false;
	}
	DroppedWidget->RemoveFromParent();
	SpacialInventoryWidget->DraggedItemWidget = nullptr;
	bIsHighlighted = false;
	InventoryComponent->OnInventoryChanged.Broadcast();
	return true;
}

FIntPoint UZInventoryGridWidget::CalculateDropPosition(const FVector2D& InMousePosition, const FIntPoint& ItemGridSize, bool bCorrectionApplied)
{
	// 1) 마우스가 그리드 상 몇 번째 타일에 있는지 구하기
	const int32 MouseTileX = FMath::FloorToInt(InMousePosition.X / TileSize);
	const int32 MouseTileY = FMath::FloorToInt(InMousePosition.Y / TileSize);

	// 2) "타일 내부"에서의 좌표 (offsetX, offsetY)를 구해서 어느 사분면인지 판단
	const float offsetX = FMath::Fmod(InMousePosition.X, TileSize);
	const float offsetY = FMath::Fmod(InMousePosition.Y, TileSize);

	// 3) 아이템을 "마우스 중심에 놓기" 위한 기본 계산
	//    (예) 3×2 아이템이라면 아이템의 절반은 (1,1)로 보고,
	//    마우스가 중앙쯤이도록 하기 위해 좌상단을 (MouseTile - (1,1))로 계산
	//    1×1처럼 홀수 크기 아이템은 CeilToInt를 쓰면 항상 한 칸 위/왼쪽으로 밀려서
	//    계산되어(예: 1의 절반을 1로 취급) 클릭한 자리와 다른 칸으로 어긋난다.
	const int32 HalfWidth = FMath::FloorToInt(ItemGridSize.X * 0.5f);
	const int32 HalfHeight = FMath::FloorToInt(ItemGridSize.Y * 0.5f);

	FIntPoint ItemDropPosition(MouseTileX - HalfWidth, MouseTileY - HalfHeight);

	// 4) 타일 내부 사분면별로 보정
	if (bCorrectionApplied)
	{
		const bool bRight = (offsetX >= TileSize * 0.25f);
		const bool bDown = (offsetY >= TileSize * 0.25f);
		ItemDropPosition.X = bRight ? ItemDropPosition.X + 1 : ItemDropPosition.X;
		ItemDropPosition.Y = bDown ? ItemDropPosition.Y + 1 : ItemDropPosition.Y;

		// 디버그 출력
		GEngine->AddOnScreenDebugMessage(-1, 0.5f, FColor::Green,
			FString::Printf(TEXT("DropPos = [%d, %d], bRight=%d, bDown=%d"),
				ItemDropPosition.X, ItemDropPosition.Y, bRight, bDown));
	}

	return ItemDropPosition;
}

bool UZInventoryGridWidget::HandleDropItem(UZInventoryItemWidget* DroppedWidget, const FVector2D& InMousePosition)
{
	if (!IsValid(DroppedWidget))
	{
		return false;
	}

	TObjectPtr<UZInventoryItem> DroppedItem = DroppedWidget->GetInventoryItem();
	if (!IsValid(DroppedItem))
	{
		return false;
	}
	
	TObjectPtr<UZEquipmentGridWidget> EquipmentGridWidget = SpacialInventoryWidget.Get()->GetEquipmentGridWidget();

	FGeometry EquipmentGridWidgetGeometry = IsValid(EquipmentGridWidget) ? EquipmentGridWidget->GetCachedGeometry() : FGeometry();
	FGeometry InventoryGridGeometry = GridCanvasPanel->GetCachedGeometry();
	bool IsInEquipWidget = IsValid(EquipmentGridWidget) && EquipmentGridWidgetGeometry.IsUnderLocation(InMousePosition);
	bool IsInGridWidget = InventoryGridGeometry.IsUnderLocation(InMousePosition);

	//TODO
	if (IsInEquipWidget && !IsInGridWidget)
	{
		UE_LOG(LogZGridWidget, Warning, TEXT("Mouse is over EquipInventoryWidget!"));
		return false;
	}
	if (!IsInEquipWidget && !IsInGridWidget)
	{
		UE_LOG(LogZGridWidget, Warning, TEXT("Mouse is over SpacialInventoryWidget!"));
		return false;
	}

	// 2) 실제 인벤토리 그리드 상 마우스 좌표 계산
	
	const FVector2D LocalMousePos = InventoryGridGeometry.AbsoluteToLocal(InMousePosition);
	FIntPoint ItemGridSize = DroppedItem->GetGridSize();
	FIntPoint ItemDropPosition = CalculateDropPosition(LocalMousePos, ItemGridSize);

	ItemDropPosition.X = FMath::Clamp(ItemDropPosition.X, 0, GridSize.X);
	ItemDropPosition.Y = FMath::Clamp(ItemDropPosition.Y, 0, GridSize.Y);

	// 2개 이상이거나(여러 아이템과 겹침) 경계를 벗어난 경우(-1) : 배치를 거부
	// 1개인 경우 : Swap
	// 0인 경우 : Add
	int32 ItemCount = InventoryComponent->GetItemCountAtPosition(DroppedItem, ItemDropPosition, ItemGridSize);
	if (ItemCount == 1) // Swap
	{
		return HandleSwap(DroppedWidget, DroppedItem, InMousePosition, ItemDropPosition);
	}
	else if (ItemCount == 0)
	{
		return HandleAddItem(DroppedWidget, DroppedItem, ItemDropPosition);
	}

	// 배치를 거부한다. false를 반환하면 StopDragging이 실패로 처리해서
	// bIsDragging이 계속 true로 남고, 아이템은 계속 마우스에 붙어 따라다닌다.
	// (여기서 true를 반환하면 위젯이 아무 데도 재배치되지 않은 채
	//  "드래그 종료"로 처리되어 인벤토리에서 사라진 것처럼 보이는 버그가 있었다.)
	return false;
}

void UZInventoryGridWidget::SetTileColorInGrid(const FVector2D& StartLocation, const FIntPoint& ItemSize, const FColor& Color)
{
	FVector2D MousePosition = GridCanvasPanel->GetCachedGeometry().AbsoluteToLocal(StartLocation);
	//TODO : 좀 더 정확하게 하려면 타일을 4등분, 지금 좌우만 나누고 상하를 안나눠서 제대로 인식안됨
	FIntPoint ItemDropPosition = CalculateDropPosition(MousePosition, ItemSize);

	if (IsItemDropHighlightOutOfBounds(ItemDropPosition, ItemSize))
	{
		bIsHighlighted = false;
		return;
	}

	//받아온 포지션은 아이템 위젯의 그리드에서 상대위치
	if (HighlightedTileLocation == StartLocation &&
		HighlightedTileSize == ItemSize &&
		HighlightedTileColor == Color)
	{
		return;
	}

	HighlightedTileLocation = ItemDropPosition * TileSize;
	HighlightedTileSize = ItemSize * TileSize;
	HighlightedTileColor = Color;

	bIsHighlighted = true;

	// 다시 그리기 요청
	Invalidate(EInvalidateWidget::Paint);
}
