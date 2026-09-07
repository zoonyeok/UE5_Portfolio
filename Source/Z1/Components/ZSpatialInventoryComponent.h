// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZSpatialInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChanged);

class UZInventoryItem;

// 네트워크 동기화를 위한 최소 데이터 전송 구조
USTRUCT(BlueprintType)
struct FInventoryUpdate
{
	GENERATED_BODY()

	FInventoryUpdate() {}

	FInventoryUpdate(FIntPoint StartPosition, FIntPoint Size, TObjectPtr<UZInventoryItem> Item, bool bAdded)
	{
		this->StartPosition = StartPosition;
		this->Size = Size;
		this->Item = Item;
		this->bAdded = bAdded;
	}

	UPROPERTY(BlueprintReadWrite)
	FIntPoint StartPosition;

	UPROPERTY(BlueprintReadWrite)
	FIntPoint Size;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UZInventoryItem> Item;

	UPROPERTY(BlueprintReadWrite)
	bool bAdded; // true: 추가, false: 삭제
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class Z1_API UZSpatialInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZSpatialInventoryComponent();

	FORCEINLINE FIntPoint GetGridSize() const { return GridSize; }

	//Item and Item Position in grid
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	TMap<UZInventoryItem*, FIntPoint> GetInventoryItems() const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 아이템 추가
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(UZInventoryItem* NewItem, FIntPoint StartPosition, bool bBroadcast = true);

	// 빈공간 찾아서 추가
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool TryAddItem(UZInventoryItem* NewItem);

	// 아이템 제거
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemByGridPosition(FIntPoint StartPosition, bool bBroadcast = true);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void RemoveItemByPointer(UZInventoryItem* ItemToRemove);

	UFUNCTION(BlueprintCallable, Category = "Grid")
	UZInventoryItem* GetItemFromGrid(FIntPoint Position) const;

	//ItemDropPosition 포지션에 아이템이 한 개 있는지, 공간이 충분한지 확인
	int32 GetItemCountAtPosition(UZInventoryItem* Item, FIntPoint ItemPosition, FIntPoint Size) const;

	// 자기 자신을 제외한 다른 아이템을 가져온다.
	TObjectPtr<UZInventoryItem> GetItemAtPosition(UZInventoryItem* Item, FIntPoint ItemPosition, FIntPoint ItemSize) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool IsItemDropPositionOutOfBounds(const FIntPoint& ItemDropPosition) const;

	UFUNCTION()
	FIntPoint FindItemPosition(UZInventoryItem* Item) const;

	UFUNCTION()
	void SwapItems(UZInventoryItem* ItemInGrid, UZInventoryItem* DroppedItem, FIntPoint DropPosition);

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
	FOnInventoryChanged OnInventoryChanged;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool CanPlaceItem(FIntPoint StartPosition, FIntPoint Size) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void SetCellsOccupied(FIntPoint StartPosition, FIntPoint Size, bool bOccupied);

	// 좌표(X, Y)가 그리드 범위 안에 있는지 확인
	bool IsPositionInBounds(int32 X, int32 Y) const;

	// StartPosition에서 Size만큼의 사각 영역이 그리드 범위 안에 있는지 확인
	bool IsRegionInBounds(FIntPoint StartPosition, FIntPoint Size) const;

	// 두 아이템이 같은 종류(같은 ItemID)인지 확인. 어느 한쪽이라도 유효하지 않으면 false.
	bool IsSameItemType(const UZInventoryItem* A, const UZInventoryItem* B) const;

	// TODO : IndexToTile
	FIntPoint IndexToPosition(int32 Index) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	int32 GetCellIndex(int32 X, int32 Y) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGrid(FIntPoint InDimensions);

	/*UFUNCTION(NetMulticast, Reliable)
	void Multicast_InventoryUpdate(const FInventoryUpdate& Update);*/

protected:
	// 현재 인벤토리에 저장된 아이템들, 좌상단에 포인터 저장
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	TArray<TObjectPtr<UZInventoryItem>> GridCells;

	// 각 셀이 점유되었는지 여부를 관리
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
	TArray<bool> SlotOccupied; // true: 점유, false: 비어 있음

	// Columns, Rows
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid", meta = (AllowPrivateAccess = "true"))
	FIntPoint GridSize;
};
