// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/ZPickable.h"
#include "ZWorldItem.generated.h"

class UZInventoryItem;
class UZWorldItemWidget;
class UWorldItemWidgetComponent;

UCLASS()
class Z1_API AZWorldItem : public AActor, public IZPickable
{
	GENERATED_BODY()
	
public:	
	AZWorldItem();

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	FName GetItemID() const { return ItemID; }

	TObjectPtr<UWorldItemWidgetComponent> GetWorldWidgetComp() const { return WorldItemWidgetComponent; }

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UZWorldItemWidget> WorldItemWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	void InitializeItem(FName NewItemID);

	void SetCollisionInSlot();

protected:
	virtual void BeginPlay() override;

	virtual EInventoryActionResult PickUp_Implementation(APawn* Player) override;

	UFUNCTION(BlueprintCallable, Category = "Pickup")
	virtual UZInventoryItem* ConvertToInventoryItem();

	bool LoadMesh();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWorldItemWidgetComponent> WorldItemWidgetComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> WorldItemWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
	FName ItemID; // Links to RowName in UDataTable

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> SkeletalMeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", Meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> ItemDataTable;
};
