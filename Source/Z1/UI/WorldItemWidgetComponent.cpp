// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/WorldItemWidgetComponent.h"
#include "UI/ZWorldItemWidget.h"
#include "Z1Character.h"
#include "Components/ZInteractComponent.h"

UWorldItemWidgetComponent::UWorldItemWidgetComponent()
{
	SetComponentTickEnabled(false);

	InteractionTime = 0.f;
	InteractionDistance = 200.f;
	InteractableNameText = FText::FromString("Interactable Object");
	InteractableActionText = FText::FromString("Interact");
	bAllowMultipleInteractors = true;

	// UI Option
	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(600, 100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true); // Default
}

void UWorldItemWidgetComponent::SetInteractableNameText(const FText& NewNameText)
{
	InteractableNameText = NewNameText;
	RefreshWidget();
}

void UWorldItemWidgetComponent::SetInteractableActionText(const FText& NewActionText)
{
	InteractableActionText = NewActionText;
	RefreshWidget();
}

void UWorldItemWidgetComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (AZ1Character* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}

	Interactors.Empty();
}

bool UWorldItemWidgetComponent::CanInteract(AZ1Character* Character) const
{
	const bool bPlayerAlreadyInteracting = !bAllowMultipleInteractors && Interactors.Num() >= 1;
	return !bPlayerAlreadyInteracting && IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UWorldItemWidgetComponent::RefreshWidget()
{
	if (!bHiddenInGame && GetOwner()->GetNetMode() != NM_DedicatedServer)
	{
		if (UZWorldItemWidget* InteractionWidget = Cast<UZWorldItemWidget>(GetUserWidgetObject()))
		{
			InteractionWidget->UpdateInteractionWidget(this);
		}
	}
}

void UWorldItemWidgetComponent::BeginFocus(AZ1Character* Character)
{
	if (!IsActive() || !IsValid(GetOwner()) || !IsValid(Character))
	{
		return;
	}

	OnBeginFocus.Broadcast(Character);

	//Draw white outlines
	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(false);

		TArray<UActorComponent*> Components;
		GetOwner()->GetComponents(UActorComponent::StaticClass(), Components);
		for (auto& Component : Components)
		{
			if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Component))
			{
				PrimitiveComp->SetRenderCustomDepth(true);
			}
		}
	}

	RefreshWidget();
}

void UWorldItemWidgetComponent::EndFocus(AZ1Character* Character)
{
	OnEndFocus.Broadcast(Character);

	//Draw white outlines
	//!GetOwner()->HasAuthority()
	if (GetNetMode() != NM_DedicatedServer)
	{
		SetHiddenInGame(true);

		TArray<UActorComponent*> Components;
		GetOwner()->GetComponents(UActorComponent::StaticClass(), Components);

		for (auto& Component : Components)
		{
			if (UPrimitiveComponent* PrimitiveComp = Cast<UPrimitiveComponent>(Component))
			{
				PrimitiveComp->SetRenderCustomDepth(false);
			}
		}
	}
}

void UWorldItemWidgetComponent::BeginInteract(AZ1Character* Character)
{
	if (CanInteract(Character))
	{
		Interactors.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}
}

void UWorldItemWidgetComponent::EndInteract(AZ1Character* Character)
{
	Interactors.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);
}

void UWorldItemWidgetComponent::Interact(AZ1Character* Character)
{
	if (CanInteract(Character))
	{
		OnInteract.Broadcast(Character);
	}
}

float UWorldItemWidgetComponent::GetInteractPercentage()
{
	if (FMath::IsNearlyZero(InteractionTime)) return 0.f;

	if (Interactors.IsValidIndex(0))
	{
		if (AZ1Character* Interactor = Interactors[0])
		{
			if (Interactor && Interactor->GetInteractionComponent() && Interactor->GetInteractionComponent()->IsInteracting())
			{
				return 1.f - FMath::Abs(Interactor->GetInteractionComponent()->GetRemainingInteractionTime() / InteractionTime);
			}
		}
	}
	return 0.f;
}
