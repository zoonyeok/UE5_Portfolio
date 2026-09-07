// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ZPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "UI/ZInventoryWidget.h"
#include "UI/ZSpacialInventoryWidget.h"
#include "UI/ZCharacterStatusWidget.h"
#include "Components/ZAttributeComponent.h"
#include "Components/ZSpatialInventoryComponent.h"
#include "Components/ZEquipmentManagerComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "Z1Character.h"

void AZPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AZPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn); //이게 없어서 카메라 세팅 고장남, 개삽질함ㄹㅇㅋㅋ
	//if (IsValid(InPawn))
	//{
	//	SetViewTargetWithBlend(InPawn, 0.0f); // 캐릭터로 카메라 설정
	//}
	InventoryWidgetInitialize();
	CharacterStatusWidgetInitialize();
}

void AZPlayerController::Destroyed()
{
	if (InventoryWidget)
	{
		InventoryWidget->RemoveFromParent();
		//InventoryWidget->RemoveFromViewport();
	}
}

void AZPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// PIE 종료/레벨 전환 시에는 Destroyed()가 호출된다는 보장이 없어서,
	// 뷰포트에 붙은 위젯들이 RemoveFromParent 없이 방치되다가 GC가 뒤늦게
	// 정리하면서 "SObjectWidget destroyed while collecting garbage" ensure가 뜨는
	// 원인이었다. EndPlay는 액터 종료 시 항상 호출되므로 여기서 확실히 떼어낸다.
	if (IsValid(InventoryWidget) && InventoryWidget->IsInViewport())
	{
		InventoryWidget->RemoveFromParent();
	}

	if (IsValid(CharacterStatusWidget) && CharacterStatusWidget->IsInViewport())
	{
		CharacterStatusWidget->RemoveFromParent();
	}

	UIOnly.SetWidgetToFocus(TSharedPtr<SWidget>());
	SetInputMode(GameOnly);
	InventoryWidget = nullptr;
	CharacterStatusWidget = nullptr;
	Super::EndPlay(EndPlayReason);
}

void AZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Local Player의 EnhancedInputSubsystem에 MappingContext 추가
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// Inventory
		EnhancedInputComponent->BindAction(InventoryAction, ETriggerEvent::Started, this, &AZPlayerController::ToggleInventory);

	}
}

void AZPlayerController::ToggleInventory()
{
	if (IsValid(InventoryWidget))
	{
		if (InventoryWidget->IsInViewport())
		{
			InventoryWidget->RemoveFromParent();
			//InventoryWidget->SetVisibility(ESlateVisibility::Visible); // 보이기
			UIOnly.SetWidgetToFocus(TSharedPtr<SWidget>());
			SetInputMode(GameOnly);
			SetShowMouseCursor(false);
		}
		else
		{
			InventoryWidget->AddToViewport();
			//InventoryWidget->SetVisibility(ESlateVisibility::Hidden); // 숨기기
			UIOnly.SetWidgetToFocus(InventoryWidget->TakeWidget()); // 포커스를 InventoryWidget에 설정
			UIOnly.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 마우스 커서를 뷰포트에 고정하지 않음
			SetInputMode(UIOnly);
			SetShowMouseCursor(true);
		}
	}
}

void AZPlayerController::InventoryWidgetInitialize()
{
	if (IsValid(InventoryWidgetClass))
	{
		InventoryWidget = CreateWidget<UZSpacialInventoryWidget>(this, InventoryWidgetClass);

		if (AZ1Character* ZCharacter = Cast<AZ1Character>(GetCharacter()))
		{
			UZSpatialInventoryComponent* SpatialInventoryComponent = ZCharacter->GetComponentByClass<UZSpatialInventoryComponent>();
			UZEquipmentManagerComponent* EquipmentComponent = ZCharacter->GetComponentByClass<UZEquipmentManagerComponent>();

			if (SpatialInventoryComponent && EquipmentComponent)
			{
				InventoryWidget->InitializeWidget(SpatialInventoryComponent, EquipmentComponent);
				InventoryWidget->SetOwningPlayer(this);
				SetShowMouseCursor(false);
				UIOnly.SetWidgetToFocus(TSharedPtr<SWidget>());
			SetInputMode(GameOnly);
			}
		}
	}
}

void AZPlayerController::CharacterStatusWidgetInitialize()
{
	if (IsValid(CharacterStatusWidgetClass))
	{
		CharacterStatusWidget = CreateWidget<UZCharacterStatusWidget>(this, CharacterStatusWidgetClass);

		if (AZ1Character* ZCharacter = Cast<AZ1Character>(GetCharacter()))
		{
			UZAttributeComponent* AttributeComponent = ZCharacter->GetComponentByClass<UZAttributeComponent>();
			CharacterStatusWidget->InitializeStatusWidget(AttributeComponent);
			CharacterStatusWidget->AddToViewport();
		}
	}
}
