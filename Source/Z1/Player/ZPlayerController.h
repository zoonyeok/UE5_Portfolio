// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ZPlayerController.generated.h"

class UZInventoryWidget;
struct FInputModeUIOnly;
struct FInputModeGameOnly;
class UInputMappingContext;
class UInputAction;
class UZSpacialInventoryWidget;
class UZCharacterStatusWidget;

/**
 *
 */
UCLASS()
class Z1_API AZPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	virtual void SetupInputComponent() override;

	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	virtual void OnPossess(APawn* InPawn) override;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InventoryAction;

	// Inventory Widget 클래스 타입
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UZSpacialInventoryWidget> InventoryWidgetClass;

	// Inventory Widget 인스턴스
	UPROPERTY()
	UZSpacialInventoryWidget* InventoryWidget;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UZCharacterStatusWidget> CharacterStatusWidgetClass;

	UPROPERTY()
	UZCharacterStatusWidget* CharacterStatusWidget;

private:
	FInputModeGameOnly GameOnly;
	FInputModeGameAndUI UIOnly;

	// 인벤토리 창 토글 함수
	void ToggleInventory();
	void InventoryWidgetInitialize();
	void CharacterStatusWidgetInitialize();
};
