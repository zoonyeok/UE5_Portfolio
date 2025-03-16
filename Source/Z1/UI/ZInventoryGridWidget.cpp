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
#include <Blueprint/WidgetLayoutLibrary.h>

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
			InventoryComponent->OnInventoryChanged.AddDynamic(this, &ThisClass::RefreshGrid);
		}
	}

	PrevItemDropPosition = FIntPoint::ZeroValue;
	InventoryItemWidgetMap.Empty();
}

void UZInventoryGridWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{

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
		UZInventoryItemWidget* ItemWidget = CreateWidget<UZInventoryItemWidget>(GetOwningPlayer(), InventoryItemWidgetClass);
		if (ItemWidget)
		{
			//SpacialInventoryWidget
			ItemWidget->InitializeItemWidget(this, Item.Key);

			if (UCanvasPanelSlot* ItemCanvasPanel = Cast<UCanvasPanelSlot>(GridCanvasPanel.Get()->AddChild(ItemWidget)))
			{
				ItemCanvasPanel->SetAutoSize(true);
				ItemCanvasPanel->SetPosition(Item.Value * TileSize);
			}

			InventoryItemWidgetMap.Add(FIntPoint(Item.Value.X, Item.Value.Y), ItemWidget);
		}
	}
}

bool UZInventoryGridWidget::IsItemDropHighlightOutOfBounds(const FIntPoint& ItemDropPosition, const FVector2D& ItemSize)
{
	FIntPoint FGridSize = FIntPoint(FMath::TruncToInt(GridSize.X), FMath::TruncToInt(GridSize.Y));
	return (ItemDropPosition.X < 0 || ItemDropPosition.X + ItemSize.X > FGridSize.X ||
		ItemDropPosition.Y < 0 || ItemDropPosition.Y + ItemSize.Y > FGridSize.Y);
}

void UZInventoryGridWidget::UpdateInventoryItemWidgetMap(UZInventoryItemWidget* ItemWidget, const FIntPoint& OldPosition, const FIntPoint& NewPosition)
{
	// 이전 위치에서 위젯을 제거
	if (InventoryItemWidgetMap.Contains(OldPosition))
	{
		InventoryItemWidgetMap.Remove(OldPosition);
	}

	// 새로운 위치에 위젯을 추가
	InventoryItemWidgetMap.Add(NewPosition, ItemWidget);
}

void UZInventoryGridWidget::HandleOutOfBoundsDrop(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem)
{
	// 1) 위젯 Viewport에서 제거
	if (DroppedWidget->IsInViewport())
	{
		DroppedWidget->RemoveFromParent();
	}

	// 2) 인벤토리에서 제거
	InventoryComponent->RemoveItemByPointer(DroppedItem);

	// 월드에 아이템 스폰
	if (SpacialInventoryWidget)
	{
		SpacialInventoryWidget->HandleDropItem(DroppedWidget);
	}
}

void UZInventoryGridWidget::HandleSwap(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem, const FVector2D& InMousePosition, const FIntPoint& ItemDropPos)
{
	// 1) 겹쳐 있는 실제 아이템 가져오기
	TObjectPtr<UZInventoryItem> ItemInGrid =
		InventoryComponent->GetItemAtPosition(DroppedItem, ItemDropPos, DroppedItem->GetGridSize());

	if (!ItemInGrid)
	{
		// 혹은 ItemCount == 1인데 실제로는 못 찾았다면 로직 오류
		return;
	}

	// 2) 아이템 위치 파악 & 대응되는 UZInventoryItemWidget 찾기
	FIntPoint FoundItemPos = InventoryComponent->FindItemPosition(ItemInGrid);
	UZInventoryItemWidget* WidgetInGrid = InventoryItemWidgetMap.FindRef(FoundItemPos);
	if (!WidgetInGrid)
	{
		// 아무것도 안 함, 혹은 LOG
		return;
	}

	// 3) 실제 인벤토리 컴포넌트에 Swap
	InventoryComponent->SwapItems(ItemInGrid, DroppedItem, ItemDropPos);

	// 4) 드롭된 위젯: Viewport에서 제거한 뒤, 그리드 패널에 붙이기
	if (DroppedWidget->IsInViewport())
	{
		DroppedWidget->RemoveFromParent();
	}

	if (UCanvasPanelSlot* GridPanel = Cast<UCanvasPanelSlot>(GridCanvasPanel->AddChild(DroppedWidget)))
	{
		GridPanel->SetAutoSize(true);
		GridPanel->SetPosition(ItemDropPos * TileSize);
	}

	WidgetInGrid->StartDragging(InMousePosition);

	// 6) 맵 갱신
	FIntPoint PrevItemPosition = InventoryComponent->FindItemPosition(DroppedItem);
	UpdateInventoryItemWidgetMap(DroppedWidget, PrevItemPosition, ItemDropPos);

	// 7) 그리드 리프레시
	RefreshGrid();
}

void UZInventoryGridWidget::HandleAddItem(UZInventoryItemWidget* DroppedWidget, TObjectPtr<UZInventoryItem> DroppedItem, const FIntPoint& ItemDropPos)
{
	if (UCanvasPanelSlot* GridPanel = Cast<UCanvasPanelSlot>(GridCanvasPanel.Get()->AddChild(DroppedWidget)))
	{
		GridPanel->SetAutoSize(true);
		GridPanel->SetPosition(ItemDropPos * TileSize);
	}

	InventoryComponent->RemoveItemByPointer(DroppedItem);

	bIsHighlighted = false;

	InventoryComponent->AddItem(DroppedItem, ItemDropPos);

	DroppedWidget->SetParentWidget(this);

	FIntPoint PrevItemPosition = InventoryComponent->FindItemPosition(DroppedItem);
	UpdateInventoryItemWidgetMap(DroppedWidget, PrevItemPosition, ItemDropPos);
}

FIntPoint UZInventoryGridWidget::CalculateDropPosition(const FVector2D& InMousePosition, const FVector2D& ItemGridSize, bool bCorrectionApplied)
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
	const int32 HalfWidth = FMath::CeilToInt(ItemGridSize.X * 0.5f);
	const int32 HalfHeight = FMath::CeilToInt(ItemGridSize.Y * 0.5f);

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
	if (!DroppedWidget)
	{
		return false;
	}

	TObjectPtr<UZInventoryItem> DroppedItem = DroppedWidget->GetInventoryItem();
	if (!DroppedItem)
	{
		return false;
	}
	
	TObjectPtr<UZEquipmentGridWidget> EquipmentGridWidget = SpacialInventoryWidget.Get()->GetEquipmentGridWidget();

	FGeometry EquipmentGridWidgetGeometry = EquipmentGridWidget.Get()->GetCachedGeometry();
	FGeometry InventoryGridGeometry = GetCachedGeometry();
	bool IsInEquipWidget = EquipmentGridWidgetGeometry.IsUnderLocation(InMousePosition);
	bool IsInGridWidget = InventoryGridGeometry.IsUnderLocation(InMousePosition);

	//TODO
	if (IsInEquipWidget && !IsInGridWidget)
	{
		UE_LOG(LogZGridWidget, Warning, TEXT("Mouse is over EquipInventoryWidge!"));
		return false;
	}
	if (!IsInEquipWidget && !IsInGridWidget)
	{
		UE_LOG(LogZGridWidget, Warning, TEXT("Mouse is over  SpacialInventoryWidge!"));
		HandleOutOfBoundsDrop(DroppedWidget, DroppedItem);
		return false;
	}

	// 2) 실제 인벤토리 그리드 상 마우스 좌표 계산
	
	const FVector2D LocalMousePos = InventoryGridGeometry.AbsoluteToLocal(InMousePosition);
	FVector2D ItemGridSize = DroppedItem->GetGridSize();
	FIntPoint ItemDropPosition = CalculateDropPosition(LocalMousePos, ItemGridSize);

	ItemDropPosition.X = FMath::Clamp(ItemDropPosition.X, 0, static_cast<int32>(GridSize.X));
	ItemDropPosition.Y = FMath::Clamp(ItemDropPosition.Y, 0, static_cast<int32>(GridSize.Y));

	// 2개 이상인 경우 : 아무일도 일어나지 않음
	// 1개인 경우 : Swap
	// 0인 경우 : Add
	int32 ItemCount = InventoryComponent->GetItemCountAtPosition(DroppedItem, ItemDropPosition, ItemGridSize);
	if (ItemCount == 1) // Swap
	{
		HandleSwap(DroppedWidget, DroppedItem, InMousePosition, ItemDropPosition);
	}
	else if (ItemCount == 0)
	{
		HandleAddItem(DroppedWidget, DroppedItem, ItemDropPosition);
	}

	return true;
}

void UZInventoryGridWidget::SetTileColorInGrid(const FVector2D& StartLocation, const FVector2D& ItemSize, const FColor& Color)
{
	FVector2D MousePosition = this->GetCachedGeometry().AbsoluteToLocal(StartLocation);
	//TODO : 좀 더 정확하게 하려면 타일을 4등분, 지금 좌우만 나누고 상하를 안나눠서 제대로 인식안됨
	FIntPoint ItemDropPosition = CalculateDropPosition(MousePosition, ItemSize, true);

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