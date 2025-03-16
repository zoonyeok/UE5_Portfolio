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

	GridSize = FVector2D(12, 5); // Default grid size
}


// Called when the game starts
void UZSpatialInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	InitializeGrid(GridSize);
}

TMap<UZInventoryItem*, FVector2D> UZSpatialInventoryComponent::GetInventoryItems() const
{
	TMap<UZInventoryItem*, FVector2D> Results;

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
				Results.Add(FoundItem, FVector2D(X, Y));
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

bool UZSpatialInventoryComponent::AddItem(UZInventoryItem* NewItem, FVector2D StartPosition, bool bBroadcast)
{
	if (!NewItem)
	{
		return false; // 배치 불가능
	}

	// 예: 아이템이 2×3이라면
	FVector2D ItemSize = NewItem->GetGridSize();

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

	if (bBroadcast && OnInventoryChanged.IsBound())
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
	FVector2D ItemSize = NewItem->GetGridSize();

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			FVector2D NewPosition(X, Y);
			if (CanPlaceItem(NewPosition, ItemSize))
			{
				AddItem(NewItem, NewPosition);
				if (OnInventoryChanged.IsBound())
				{
					OnInventoryChanged.Broadcast();
				}
				return true;
			}
		}
	}

	return false;
}

void UZSpatialInventoryComponent::RemoveItemByGridPosition(FVector2D StartPosition, bool bBroadcast)
{
	int32 StartIndex = GetCellIndex(StartPosition.X, StartPosition.Y);
	UZInventoryItem* ItemToRemove = GridCells[StartIndex];
	if (ItemToRemove == nullptr)
	{
		//LOG
		return; // No item to remove
	}

	FVector2D ItemSize = ItemToRemove->GetGridSize();

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
	if (bBroadcast && OnInventoryChanged.IsBound())
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
	if (FoundPos.X < 0 || FoundPos.Y < 0  || FoundPos.X > GridSize.X || FoundPos.Y > GridSize.Y)
	{
		// 해당 아이템이 그리드에 없는 경우
		return;
	}

	// 위에서 찾은 위치를 기반으로 RemoveItemByGridPosition 호출
	RemoveItemByGridPosition(FVector2D(FoundPos.X, FoundPos.Y));
}

UZInventoryItem* UZSpatialInventoryComponent::GetItemFromGrid(FVector2D Position) const
{
	int32 Index = GetCellIndex(Position.X, Position.Y);
	// 경계 초과 확인
	if (Index < 0 || Index >= GridCells.Num())
	{
		return nullptr; // 경계를 초과
	}
	return GridCells[Index];
}

//TODO : 지금
int32 UZSpatialInventoryComponent::GetItemCountAtPosition(UZInventoryItem* Item, FVector2D ItemPosition, FVector2D ItemSize) const
{
	int32 StartY = ItemPosition.Y;
	int32 StartX = ItemPosition.X;
	int32 EndY = StartY + ItemSize.Y;
	int32 EndX = StartX + ItemSize.X;

	// 경계 초과 확인으로 공간확인
	if (StartX < 0 || StartY < 0 || EndX > GridSize.X || EndY > GridSize.Y)
	{
		return -1; // 경계를 초과
	}

	//아이템 종류확인
	TSet<FName> ItemNum;
	TObjectPtr<UZInventoryItem> FoundItem;
	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			TObjectPtr<UZInventoryItem> CellItem = GridCells[Index];

			if (IsValid(CellItem))
			{
				FItemStaticData* CellItemData = CellItem.Get()->GetStaticData();
				FItemStaticData* OriginItemData = Item->GetStaticData();
				if (CellItemData && OriginItemData)
				{
					//자기 자신 제외
					if (CellItemData->ItemID == OriginItemData->ItemID)
					{
						continue;
					}
				}

				if (CellItemData && CellItemData->ItemID.ToString().Len() > 0)
				{
					if (!ItemNum.Find(CellItemData->ItemID))
					{
						ItemNum.Add(CellItemData->ItemID);
					}
				}
			}
		}
	}

	return ItemNum.Num();
}

TObjectPtr<UZInventoryItem> UZSpatialInventoryComponent::GetItemAtPosition(UZInventoryItem* Item, FVector2D ItemPosition, FVector2D ItemSize) const
{
	int32 StartY = ItemPosition.Y;
	int32 StartX = ItemPosition.X;
	int32 EndY = StartY + ItemSize.Y;
	int32 EndX = StartX + ItemSize.X;

	// 경계 초과 확인
	if (StartX < 0 || StartY < 0 || EndX > GridSize.X || EndY > GridSize.Y)
	{
		return nullptr; // 경계 초과
	}

	for (int32 Y = StartY; Y < EndY; ++Y)
	{
		for (int32 X = StartX; X < EndX; ++X)
		{
			int32 Index = GetCellIndex(X, Y);
			TObjectPtr<UZInventoryItem> CellItem = GridCells[Index];

			if (IsValid(CellItem))
			{
				FItemStaticData* CellItemData = CellItem.Get()->GetStaticData();
				FItemStaticData* OriginItemData = Item->GetStaticData();
				if (CellItemData && OriginItemData)
				{
					//자기 자신 제외
					if (CellItemData->ItemID == OriginItemData->ItemID)
					{
						continue;
					}
				}
				return CellItem; // 첫 번째 유효한 아이템 반환
			}
		}
	}

	return nullptr;
}

bool UZSpatialInventoryComponent::IsItemDropPositionOutOfBounds(const FIntPoint& ItemDropPosition) const
{
	return (ItemDropPosition.X < 0 || ItemDropPosition.X > static_cast<int32>(GridSize.X) ||
		ItemDropPosition.Y < 0 || ItemDropPosition.Y > static_cast<int32>(GridSize.Y));
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
	
	// 마우스로 클릭하고 아직 그리드에 안넣어준 상태
	// DroppedItemPosition이 음수가되는 이유는 마우스에 고정되어있고 그리드에 없기 때문
	if (DroppedItemPosition.X < 0 && DroppedItemPosition.Y < 0)
	{
		RemoveItemByGridPosition(FVector2D(ItemInGridPosition.X, ItemInGridPosition.Y), false);
		AddItem(DroppedItem, FVector2D(DropPosition.X, DropPosition.Y), false);
		if (OnInventoryChanged.IsBound())
		{
			OnInventoryChanged.Broadcast();
		}
	}

	if (ItemInGridPosition.X >= 0 && ItemInGridPosition.Y >= 0 && DroppedItemPosition.X >= 0 && DroppedItemPosition.Y >= 0)
	{
		// 기존 위치에서 아이템 제거
		RemoveItemByGridPosition(FVector2D(ItemInGridPosition.X, ItemInGridPosition.Y), false);
		RemoveItemByGridPosition(FVector2D(DroppedItemPosition.X, DroppedItemPosition.Y), false);

		AddItem(DroppedItem, FVector2D(DropPosition.X, DropPosition.Y), false);
		if (OnInventoryChanged.IsBound())
		{
			OnInventoryChanged.Broadcast();
		}
	}
}

bool UZSpatialInventoryComponent::CanPlaceItem(FVector2D StartPosition, FVector2D Size) const
{
	int32 StartY = StartPosition.Y;
	int32 StartX = StartPosition.X;

	int32 EndY = StartY + Size.Y;
	int32 EndX = StartX + Size.X;

	//TODO + Size해줘야하나 
	// 경계 초과 확인
	if (StartX < 0 || StartY < 0 || EndX > GridSize.X || EndY > GridSize.Y)
	{
		return false;
	}

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

void UZSpatialInventoryComponent::SetCellsOccupied(FVector2D StartPosition, FVector2D Size, bool bOccupied)
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
	int32 X = Index % static_cast<int32>(GridSize.X); // 가로 너비를 나머지 연산
	int32 Y = Index / static_cast<int32>(GridSize.X); // 가로 너비로 나눠서 행 계산

	return FIntPoint(X, Y);
}

int32 UZSpatialInventoryComponent::GetCellIndex(int32 X, int32 Y) const
{
	return Y * GridSize.X + X;
}

void UZSpatialInventoryComponent::InitializeGrid(FVector2D InDimensions)
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




