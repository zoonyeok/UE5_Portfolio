// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ZGameTypes.h"
#include "ZInventoryItem.generated.h"

class UMaterialInstanceDynamic;
/**
 * 
 */
UCLASS(BlueprintType, Blueprintable)
class Z1_API UZInventoryItem : public UObject
{
	GENERATED_BODY()
	
public:
    /** Initialize the inventory item */
    UFUNCTION(BlueprintCallable, Category = "Inventory Item")
    void Initialize(UDataTable* DataTable, FName RowName, int32 InitialQuantity);

    /** Retrieve the static item data */
    FItemStaticData* GetStaticData() const;

    /** Get the dynamic item instance data */
    const FItemInstanceData& GetInstanceData() const { return InstanceData; }

    FORCEINLINE FVector2D GetGridSize() const { return ItemGridSize; }

    UMaterialInstanceDynamic* GetIconMaterial(bool Rotate);

    //TODO
    virtual bool SpawnWorldItem();

protected:
    UMaterialInstanceDynamic* CreateItemIconDynamicMaterial();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UUserWidget> InvenWidget;

    /** Reference to the DataTable containing static item data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", Meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> ItemDataTable;

    /** Row Name for identifying the item in the DataTable */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", Meta = (AllowPrivateAccess = "true"))
    FName ItemRowName;

    // 아이템의 크기 (그리드 셀 크기)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector2D ItemGridSize; // 예: 2x2 아이템

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UMaterialInstanceDynamic* IconMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsRotate;

    /** Dynamic instance-specific data */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item", Meta = (AllowPrivateAccess = "true"))
    FItemInstanceData InstanceData;

    /** Cached static data for performance */
    mutable FItemStaticData CachedStaticData;
    mutable bool bIsStaticDataCached;
};
