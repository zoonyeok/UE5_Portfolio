// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZActionComponent.h"
#include "Actions/ZAction.h"
#include "Actions/ZRollAction.h"
#include "ZGameplayTags.h"

// Sets default values for this component's properties
UZActionComponent::UZActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	//DefaultActions.Add(UZRollAction::StaticClass());
}

void UZActionComponent::BeginPlay()
{
	Super::BeginPlay();

	for (auto ActionClass : DefaultActions)
	{
		AddAction(GetOwner(), ActionClass);
	}
}

void UZActionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FString DebugMsg = GetNameSafe(GetOwner()) + " : " + ActiveGameplayTags.ToStringSimple();
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::White, DebugMsg);

	for (const auto& ActionTuple : ActionsTagMap)
	{
		if (const TObjectPtr<UZAction>& Action = ActionTuple.Value)
		{
			if (Action->IsBeUpdated())
			{
				Action->UpdateAction(GetOwner(), DeltaTime);
			}
		}
	}
}

void UZActionComponent::AddAction(AActor* Instigator, TSubclassOf<UZAction> ActionClass)
{
	if (!ensure(ActionClass))
	{
		return;
	}

	UZAction* NewAction = NewObject<UZAction>(GetOwner(), ActionClass);
	if (IsValid(NewAction))
	{
		NewAction->Initialize(this);

		ActionsMap.Add(NewAction->ActionName, NewAction);

		ActionsTagMap.Add(NewAction->ActivationTag, NewAction);

		if (NewAction->bAutoStart && NewAction->CanStartAction(GetOwner()))
		{
			NewAction->StartAction(Instigator);
		}
	}
}

bool UZActionComponent::RemoveAction(FName ActionName)
{
	if (!ActionsMap.Contains(ActionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No ActionName in ActionMaps"));
		return false;
	}

	ActionsMap.Remove(ActionName);
	return true;
}

bool UZActionComponent::StartActionByName(AActor* Instigator, FName ActionName)
{
 	if (!ActionsMap.Contains(ActionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No ActionName in ActionMaps"));
		return false;
	}

	UZAction* FoundAction = ActionsMap.FindRef(ActionName);
	if (!IsValid(FoundAction))
	{
		return false;
	}

	if (!FoundAction->CanStartAction(GetOwner()))
	{
		return false;
	}
	
	// if Client
	/*if (!GetOwner()->HasAuthority())
	{
		ServerStartAction(Instigator, ActionName);
	}*/

	FoundAction->StartAction(Instigator);
	return true;
}

bool UZActionComponent::StopActionByName(AActor* Instigator, FName ActionName)
{
	if (!ActionsMap.Contains(ActionName))
	{
		UE_LOG(LogTemp, Warning, TEXT("No ActionName in ActionMaps"));
		return false;
	}

	UZAction* FoundAction = ActionsMap.FindRef(ActionName);
	if (!IsValid(FoundAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to run: %s"), *ActionName.ToString());
		return false;
	}

	if (!FoundAction->IsRunning())
	{
		return false;
	}

	FoundAction->StopAction(Instigator);
	return true;
}

bool UZActionComponent::StartActionByTag(AActor* Instigator, FGameplayTag ActionTag)
{
	if (!ActionsTagMap.Contains(ActionTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("No ActionName in ActionMaps"));
		return false;
	}

	UZAction* FoundAction = ActionsTagMap.FindRef(ActionTag);
	if (!IsValid(FoundAction))
	{
		return false;
	}

	if (!FoundAction->CanStartAction(GetOwner()))
	{
		return false;
	}

	// if Client
	/*if (!GetOwner()->HasAuthority())
	{
		ServerStartAction(Instigator, ActionName);
	}*/



	FoundAction->StartAction(Instigator);
	return true;
}

bool UZActionComponent::StopActionByTag(AActor* Instigator, FGameplayTag ActionTag)
{
	if (!ActionsTagMap.Contains(ActionTag))
	{
		UE_LOG(LogTemp, Warning, TEXT("No ActionName in ActionMaps"));
		return false;
	}

	UZAction* FoundAction = ActionsTagMap.FindRef(ActionTag);
	if (!IsValid(FoundAction))
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to run: %s"), *ActionTag.ToString());
		return false;
	}

	if (!FoundAction->IsRunning())
	{
		return false;
	}
	
	FoundAction->StopAction(Instigator);
	return true;
}

UZAction* UZActionComponent::FindActionByName(FName ActionName)
{
	if (ActionsMap.Contains(ActionName))
	{
		UZAction* FoundAction = ActionsMap.FindRef(ActionName);
		if (IsValid(FoundAction))
		{
			return FoundAction;
		}
	}
	return nullptr;
}

UZAction* UZActionComponent::FindActionByTag(FGameplayTag TagName)
{
	if (ActionsTagMap.Contains(TagName))
	{
		UZAction* FoundAction = ActionsTagMap.FindRef(TagName);
		if (IsValid(FoundAction))
		{
			return FoundAction;
		}
	}
	return nullptr;
}
