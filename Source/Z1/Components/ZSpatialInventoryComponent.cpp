// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZSpatialInventoryComponent.h"
#include "Items/ZInventoryItem.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UZSpatialInventoryComponent::UZSpatialInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	GridSize = FIntPoint(12, 5); // Default grid size
}


// Called when the game starts
void UZSpatialInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	InitializeGrid(GridSize);
}

TMap<UZInventoryItem*, FIntPoint> UZSpatialInventoryComponent::GetInventoryItems() const
{
	TMap<UZInventoryItem*, FIntPoint> Results;

	if (GridCells.Num() <= 0)
	{
		return Results;
	}

	int32 StartY = 0;
	int32 StartX = 0;
	int32 EndY = GridSize.Y;
	int32 EndX = GridSize.X;

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			UZInventoryItem* FoundItem = GridCells[Index];

			// 유효한 아이템 포인터가 있고, 아직 Map에 없는 아이템이면 => 추가
			if (FoundItem != nullptr && !Results.Contains(FoundItem))
			{
				// 첫 발견된 좌표를 기록 (보통 좌상단이 됨)
				Results.Add(FoundItem, FIntPoint(X, Y));
			}
		}
	}

	return Results;
}

// Replication setup
void UZSpatialInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, GridCells);
	DOREPLIFETIME(ThisClass, SlotOccupied);
}

bool UZSpatialInventoryComponent::AddItem(UZInventoryItem* NewItem, FIntPoint StartPosition, bool bBroadcast)
{
	if (!NewItem)
	{
		return false; // 배치 불가능
	}

	// 예: 아이템이 2×3이라면
	FIntPoint ItemSize = NewItem->GetGridSize();
	// 먼저 배치가 가능한지(경계 밖 or 이미 점유된 셀이 있는지) 확인
	if (!CanPlaceItem(StartPosition, ItemSize))
	{
		return false;
	}

	// 실제로 해당 영역의 모든 셀을 "NewItem" 포인터로 채움
	for (int32 Y = StartPosition.Y; Y < StartPosition.Y + ItemSize.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < StartPosition.X + ItemSize.X; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			GridCells[Index] = NewItem;      // 모든 셀에 같은 아이템 포인터
			SlotOccupied[Index] = true;      // 점유 여부도 true
		}
	}

	if (bBroadcast)
	{
		OnInventoryChanged.Broadcast();
	}

	return true;
}

bool UZSpatialInventoryComponent::TryAddItem(UZInventoryItem* NewItem)
{
	if (!NewItem)
	{
		return false;
	}

	int32 StartY = 0;
	int32 StartX = 0;
	int32 EndY = GridSize.Y;
	int32 EndX = GridSize.X;
	FIntPoint ItemSize = NewItem->GetGridSize();

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			FIntPoint NewPosition(X, Y);
			if (CanPlaceItem(NewPosition, ItemSize))
			{
				// AddItem이 자체적으로 브로드캐스트를 처리하므로 여기서 다시 호출하지 않는다.
				return AddItem(NewItem, NewPosition);
			}
		}
	}

	return false;
}

void UZSpatialInventoryComponent::RemoveItemByGridPosition(FIntPoint StartPosition, bool bBroadcast)
{
	if (!IsPositionInBounds(StartPosition.X, StartPosition.Y))
	{
		return; // 유효하지 않은 위치
	}

	int32 StartIndex = GetCellIndex(StartPosition.X, StartPosition.Y);
	UZInventoryItem* ItemToRemove = GridCells[StartIndex];
	if (ItemToRemove == nullptr)
	{
		//LOG
		return; // No item to remove
	}

	FIntPoint ItemSize = ItemToRemove->GetGridSize();

	// 해당 영역을 "비움" (nullptr, false)
	for (int32 Y = StartPosition.Y; Y < StartPosition.Y + ItemSize.Y; ++Y)
	{
		for (int32 X = StartPosition.X; X < StartPosition.X + ItemSize.X; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			GridCells[Index] = nullptr;
			SlotOccupied[Index] = false;
		}
	}

	// Broadcast the update
	//Multicast_InventoryUpdate(FInventoryUpdate(StartPosition, Size, ItemToRemove, false));
	if (bBroadcast)
	{
		OnInventoryChanged.Broadcast();
	}

	//LOG
}

void UZSpatialInventoryComponent::RemoveItemByPointer(UZInventoryItem* ItemToRemove)
{
	if (!IsValid(ItemToRemove))
	{
		return;
	}

	// 아이템의 "좌상단 셀" 혹은 "가장 먼저 발견된 셀" 위치를 찾는다
	FIntPoint FoundPos = FindItemPosition(ItemToRemove);
	if (!IsPositionInBounds(FoundPos.X, FoundPos.Y))
	{
		// 해당 아이템이 그리드에 없는 경우
		return;
	}

	// 위에서 찾은 위치를 기반으로 RemoveItemByGridPosition 호출
	RemoveItemByGridPosition(FoundPos);
}

UZInventoryItem* UZSpatialInventoryComponent::GetItemFromGrid(FIntPoint Position) const
{
	// Position.X/Y만 개별로 검사하지 않고 GetCellIndex부터 계산하면,
	// 음수 좌표가 옆 행의 유효한 인덱스로 alias되어 잘못된 아이템을 반환할 수 있다.
	if (!IsPositionInBounds(Position.X, Position.Y))
	{
		return nullptr; // 경계를 초과
	}

	return GridCells[GetCellIndex(Position.X, Position.Y)];
}

//TODO : 지금
int32 UZSpatialInventoryComponent::GetItemCountAtPosition(UZInventoryItem* Item, FIntPoint ItemPosition, FIntPoint ItemSize) const
{
	if (!IsRegionInBounds(ItemPosition, ItemSize))
	{
		return -1; // 경계를 초과
	}

	int32 StartY = ItemPosition.Y;
	int32 StartX = ItemPosition.X;
	int32 EndY = StartY + ItemSize.Y;
	int32 EndX = StartX + ItemSize.X;

	//아이템 종류확인 (자기 자신과 같은 종류는 제외)
	TSet<UZInventoryItem*> ItemNum;
	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			TObjectPtr<UZInventoryItem> CellItem = GridCells[Index];

			if (!IsValid(CellItem) || CellItem == Item)
			{
				continue;
			}

			ItemNum.Add(CellItem.Get());


		}
	}

	return ItemNum.Num();
}

TObjectPtr<UZInventoryItem> UZSpatialInventoryComponent::GetItemAtPosition(UZInventoryItem* Item, FIntPoint ItemPosition, FIntPoint ItemSize) const
{
	if (!IsRegionInBounds(ItemPosition, ItemSize))
	{
		return nullptr; // 경계 초과
	}

	int32 StartY = ItemPosition.Y;
	int32 StartX = ItemPosition.X;
	int32 EndY = StartY + ItemSize.Y;
	int32 EndX = StartX + ItemSize.X;

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			TObjectPtr<UZInventoryItem> CellItem = GridCells[Index];

			if (IsValid(CellItem) && CellItem != Item)
			{
				return CellItem; // 자기 자신이 아닌 첫 번째 유효한 아이템 반환
			}
		}
	}

	return nullptr;
}

bool UZSpatialInventoryComponent::IsItemDropPositionOutOfBounds(const FIntPoint& ItemDropPosition) const
{
	return !IsPositionInBounds(ItemDropPosition.X, ItemDropPosition.Y);
}

FIntPoint UZSpatialInventoryComponent::FindItemPosition(UZInventoryItem* Item) const
{
	for (int32 Index = 0; Index < GridCells.Num(); ++Index)
	{
		if (GridCells[Index] == Item)
		{
			return IndexToPosition(Index);
		}
	}
	return FIntPoint(-1, -1); // 아이템이 없는 경우
}

void UZSpatialInventoryComponent::SwapItems(UZInventoryItem* ItemInGrid, UZInventoryItem* DroppedItem, FIntPoint DropPosition)
{
	if (!ItemInGrid || !DroppedItem)
	{
		return;
	}

	FIntPoint ItemInGridPosition = FindItemPosition(ItemInGrid);
	FIntPoint DroppedItemPosition = FindItemPosition(DroppedItem);

	// ItemInGrid가 있던 자리를 비운다. (위젯 쪽에서 이 아이템을 커서로 픽업 처리)
	RemoveItemByGridPosition(ItemInGridPosition, false);

	// 마우스로 집고 아직 그리드에 안 넣어준 상태라면 DroppedItemPosition이 (-1, -1)이라 여기 건너뜀.
	// DroppedItem이 이미 이 그리드 안에 있었다면(같은 인벤토리 내 재배치) 원래 자리도 비운다.
	if (DroppedItemPosition.X >= 0 && DroppedItemPosition.Y >= 0)
	{
		RemoveItemByGridPosition(DroppedItemPosition, false);
	}

	AddItem(DroppedItem, DropPosition, false);

	OnInventoryChanged.Broadcast();
}

bool UZSpatialInventoryComponent::CanPlaceItem(FIntPoint StartPosition, FIntPoint Size) const
{
	if (!IsRegionInBounds(StartPosition, Size))
	{
		return false;
	}

	int32 StartY = StartPosition.Y;
	int32 StartX = StartPosition.X;

	int32 EndY = StartY + Size.Y;
	int32 EndX = StartX + Size.X;

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			const int32 Index = GetCellIndex(X, Y);
			if (!SlotOccupied.IsValidIndex(Index) || SlotOccupied[Index] == true)
			{
				return false;
			}
		}
	}
	return true;
}

void UZSpatialInventoryComponent::SetCellsOccupied(FIntPoint StartPosition, FIntPoint Size, bool bOccupied)
{
	int32 StartX = StartPosition.X;
	int32 StartY = StartPosition.Y;
	int32 EndX = StartX + Size.X;
	int32 EndY = StartY + Size.Y;

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			//TODO : INDEX CHECK
			
			SlotOccupied[Index] = bOccupied;
		}
	}
}

FIntPoint UZSpatialInventoryComponent::IndexToPosition(int32 Index) const
{
	// 예외 처리: 잘못된 Index일 경우
	if (Index < 0 || Index >= GridCells.Num())
	{
		UE_LOG(LogTemp, Warning, TEXT("IndexToPosition: Index %d is out of bounds."), Index);
		return FIntPoint(-1, -1); // 유효하지 않은 좌표 반환
	}

	// X와 Y를 계산
	int32 X = Index % GridSize.X; // 가로 너비를 나머지 연산
	int32 Y = Index / GridSize.X; // 가로 너비로 나눠서 행 계산

	return FIntPoint(X, Y);
}

int32 UZSpatialInventoryComponent::GetCellIndex(int32 X, int32 Y) const
{
	return Y * GridSize.X + X;
}

bool UZSpatialInventoryComponent::IsPositionInBounds(int32 X, int32 Y) const
{
	return X >= 0 && Y >= 0 && X < GridSize.X && Y < GridSize.Y;
}

bool UZSpatialInventoryComponent::IsRegionInBounds(FIntPoint StartPosition, FIntPoint Size) const
{
	const int32 EndX = StartPosition.X + Size.X;
	const int32 EndY = StartPosition.Y + Size.Y;

	return StartPosition.X >= 0 && StartPosition.Y >= 0
		&& EndX <= GridSize.X && EndY <= GridSize.Y;
}

bool UZSpatialInventoryComponent::IsSameItemType(const UZInventoryItem* A, const UZInventoryItem* B) const
{
	if (!A || !B)
	{
		return false;
	}

	const FItemStaticData* DataA = A->GetStaticData();
	const FItemStaticData* DataB = B->GetStaticData();

	return DataA && DataB && DataA->ItemID == DataB->ItemID;
}

void UZSpatialInventoryComponent::InitializeGrid(FIntPoint InDimensions)
{
	GridSize = InDimensions;
	int32 TotalCells = GridSize.X * GridSize.Y;

	GridCells.SetNum(TotalCells);
	SlotOccupied.SetNum(TotalCells);

	for (int32 i = 0; i < TotalCells; ++i)
	{
		GridCells[i] = nullptr;
		SlotOccupied[i] = false;
	}
}

//void UZSpatialInventoryComponent::Multicast_InventoryUpdate(const FInventoryUpdate& Update)
//{
//	// 클라이언트에서 업데이트 처리
//	if (Update.bAdded)
//	{
//		AddItem(Update.Item, Update.StartPosition);
//	}
//	else
//	{
//		RemoveItem(Update.StartPosition);
//	}
//}




