// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZActionComponent.h"
#include "Actions/ZAction.h"
#include "Actions/ZRollAction.h"
#include "ZGameplayTags.h"
#include "Engine/Engine.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UZActionComponent::UZActionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// Enable replication
	SetIsReplicatedByDefault(true);

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

void UZActionComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UZActionComponent, ActiveGameplayTags);
	DOREPLIFETIME(UZActionComponent, ReplicatedRunningActions);
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

	// Network handling
	if (GetOwner()->HasAuthority())
	{
		// Server: Execute directly
		StartActionInternal(Instigator, FoundAction);
	}
	else
	{
		// Client: Send RPC to server
		ServerStartActionByName(ActionName);
	}

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

	// Network handling
	if (GetOwner()->HasAuthority())
	{
		// Server: Execute directly
		StopActionInternal(Instigator, FoundAction);
	}
	else
	{
		// Client: Send RPC to server
		ServerStopActionByName(ActionName);
	}

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

	// Network handling
	if (GetOwner()->HasAuthority())
	{
		// Server: Execute directly
		StartActionInternal(Instigator, FoundAction);
	}
	else
	{
		// Client: Send RPC to server
		ServerStartActionByTag(ActionTag);
	}

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

	// Network handling
	if (GetOwner()->HasAuthority())
	{
		// Server: Execute directly
		StopActionInternal(Instigator, FoundAction);
	}
	else
	{
		// Client: Send RPC to server
		ServerStopActionByTag(ActionTag);
	}

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

// Network RPC Implementations
void UZActionComponent::ServerStartActionByName_Implementation(FName ActionName)
{
	if (UZAction* Action = FindActionByName(ActionName))
	{
		if (CanStartActionOnServer(GetOwner(), Action))
		{
			StartActionInternal(GetOwner(), Action);
		}
	}
}

bool UZActionComponent::ServerStartActionByName_Validate(FName ActionName)
{
	return !ActionName.IsNone();
}

void UZActionComponent::ServerStopActionByName_Implementation(FName ActionName)
{
	if (UZAction* Action = FindActionByName(ActionName))
	{
		if (Action->IsRunning())
		{
			StopActionInternal(GetOwner(), Action);
		}
	}
}

bool UZActionComponent::ServerStopActionByName_Validate(FName ActionName)
{
	return !ActionName.IsNone();
}

void UZActionComponent::ServerStartActionByTag_Implementation(FGameplayTag ActionTag)
{
	if (UZAction* Action = FindActionByTag(ActionTag))
	{
		if (CanStartActionOnServer(GetOwner(), Action))
		{
			StartActionInternal(GetOwner(), Action);
		}
	}
}

bool UZActionComponent::ServerStartActionByTag_Validate(FGameplayTag ActionTag)
{
	return ActionTag.IsValid();
}

void UZActionComponent::ServerStopActionByTag_Implementation(FGameplayTag ActionTag)
{
	if (UZAction* Action = FindActionByTag(ActionTag))
	{
		if (Action->IsRunning())
		{
			StopActionInternal(GetOwner(), Action);
		}
	}
}

bool UZActionComponent::ServerStopActionByTag_Validate(FGameplayTag ActionTag)
{
	return ActionTag.IsValid();
}

void UZActionComponent::OnRep_ActiveGameplayTags()
{
	// Handle gameplay tag changes on clients
	FGameplayTagContainer AddedTags = ActiveGameplayTags.Filter(PreviousActiveGameplayTags);
	FGameplayTagContainer RemovedTags = PreviousActiveGameplayTags.Filter(ActiveGameplayTags);

	// Update previous state
	PreviousActiveGameplayTags = ActiveGameplayTags;

	// Notify about tag changes if needed
	UE_LOG(LogTemp, Log, TEXT("ActiveGameplayTags replicated. Added: %s, Removed: %s"), 
		*AddedTags.ToStringSimple(), *RemovedTags.ToStringSimple());
}

// Network Helper Functions
bool UZActionComponent::IsActionValidForNetworking(UZAction* Action) const
{
	return IsValid(Action) && !Action->ActionName.IsNone();
}

bool UZActionComponent::CanStartActionOnServer(AActor* Instigator, UZAction* Action) const
{
	if (!IsActionValidForNetworking(Action))
	{
		return false;
	}

	// Server-side validation
	return Action->CanStartAction(Instigator);
}

void UZActionComponent::StartActionInternal(AActor* Instigator, UZAction* Action)
{
	if (!IsValid(Action)) return;

	Action->StartAction(Instigator);

	// Update replicated state
	if (GetOwner()->HasAuthority())
	{
		ReplicatedRunningActions.AddUnique(Action->ActionName);
	}

	// Broadcast event
	OnActionStarted.Broadcast(this, Action);
}

void UZActionComponent::StopActionInternal(AActor* Instigator, UZAction* Action)
{
	if (!IsValid(Action)) return;

	Action->StopAction(Instigator);

	// Update replicated state
	if (GetOwner()->HasAuthority())
	{
		ReplicatedRunningActions.Remove(Action->ActionName);
	}

	// Broadcast event
	OnActionStopped.Broadcast(this, Action);
}
