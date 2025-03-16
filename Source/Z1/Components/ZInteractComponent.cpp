// Fill out your copyright notice in the Description page of Project Settings.


#include "ZInteractComponent.h"

#include "DrawDebugHelpers.h"
#include "Z1Character.h"
#include "Interfaces/ZInteractable.h"
#include "Interfaces/ZPickable.h"
#include "UI/ZInteractWidget.h"
#include "UI/ZWorldItemWidget.h"
#include "Items/ZWorldItem.h"
#include "Kismet/GameplayStatics.h"
#include "UI/WorldItemWidgetComponent.h"
#include "TimerManager.h"

DEFINE_LOG_CATEGORY_STATIC(LogZInteractionComp, All, All)

bool bDebugDraw = false; 
static TAutoConsoleVariable<bool> CVarDebugDrawInteraction(
	TEXT("z.InteractionDebugDraw"),
	bDebugDraw,
	TEXT("Enable Debug Lines for Interact Component."),
	ECVF_Default // Cheat
);

UZInteractComponent::UZInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TraceDistance = 1000.f;
	TraceRadius = 30.f;
	//TODO
	//CollisionChannel = ;
}

void UZInteractComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UZInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const bool bIsInteractingOnServer = GetOwner()->HasAuthority() && IsInteracting();

	//Multiplayer CODE
	/*if ((!GetOwner()->HasAuthority() || bIsInteractingOnServer) && GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}*/

	// 기존 FocusedActor에 대한 해제 처리 함수
	// 새로운 FocusedActor 탐색
	// 기존 FocusedActor 해제 (라인 제거, 위젯 제거 등)
	// 새로운 FocusedActor 설정 (라인 표시, 위젯 추가 등)
	// UI update

	AZWorldItem* CurrentWorldItem = GetCurrentWorldItem();
	if (IsValid(CurrentWorldItem))
	{
		if (IsActorOutOfRange(CurrentWorldItem) || !IsActorInSight(CurrentWorldItem))
		{
			// 기존 FocusedActor에 대한 해제 처리 함수
			CouldntFindInteractable();
		}
	}
	if (GetWorld()->TimeSince(InteractionData.LastInteractionCheckTime) > InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
}

FInteractionResult* UZInteractComponent::FindInteractableActor()
{
	FInteractionResult* Result = new FInteractionResult();

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	AZ1Character* MyOwner = Cast<AZ1Character>(GetOwner());
	FVector EyeLocation;
	FRotator EyeRotation;
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector End = EyeLocation + (EyeRotation.Vector() * TraceDistance);

	TArray<FHitResult> Hits;
	FCollisionShape Shape;
	Shape.SetSphere(TraceRadius);

	const bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, EyeLocation, End, FQuat::Identity, ObjectQueryParams, Shape);

	for (FHitResult& Hit : Hits)
	{
		AActor* HitActor = Hit.GetActor();
		if (IsValid(HitActor))
		{
			if (HitActor->Implements<UZPickable>())
			{
				Result->InventoryInterface = TScriptInterface<UZPickable>(HitActor);
				Result->InteractionType = EInteractionType::Pickup;
			}
			if (HitActor->Implements<UZInteractable>())
			{
				Result->ItemInteractInterface = TScriptInterface<UZInteractable>(HitActor);

				// 만약 이미 주울 수 있는 아이템으로 판별됐다면, InteractionType을 다르게 표시
				if (Result->InteractionType == EInteractionType::Pickup)
				{
					Result->InteractionType = EInteractionType::Both; // 상호작용과 획득 모두 해당됨
				}
				else
				{
					Result->InteractionType = EInteractionType::Interact; // 상호작용 가능한 Prop
				}
			}

			if (Result->InteractionType != EInteractionType::None)
			{
				Result->Actor = HitActor;
				return Result; // 첫 번째로 찾은 상호작용 대상을 반환
			}
			Result->Actor = HitActor;
		}
	}

	if (CVarDebugDrawInteraction.GetValueOnGameThread())
	{
		DrawDebugInfo(EyeLocation, End, Hits, Result != nullptr);
	}

	return Result;
}

void UZInteractComponent::DrawDebugInfo(const FVector& Start, const FVector& End, const TArray<FHitResult>& Hits, bool bBlockingHit) const
{
	//bool bDebugDraw = CVarDebugDrawInteraction.GetValueOnGameThread();
	if (!bDebugDraw) return;

	const FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;
	for (const FHitResult& Hit : Hits)
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 32, LineColor, false, 1.0f);
	}

	DrawDebugLine(GetWorld(), Start, End, LineColor, false, 1.0f, 0, 1.0f);
}

void UZInteractComponent::BeginInteract()
{
	if (!GetOwner()->HasAuthority())
	{
		ServerBeginInteract();
	}

	InteractionData.bInteractHeld = true;

	if (UWorldItemWidgetComponent* ItemComponent = GetCurrentWorldItemWidgetComponent())
	{
		AZ1Character* Character = Cast<AZ1Character>(GetOwner());
		if (!Character) return;

		ItemComponent->BeginInteract(Character);

		if (FMath::IsNearlyZero(ItemComponent->InteractionTime))
		{
			Interact();
		}
		else //hold E key to interact
		{
			//GetWorld()->GetTimerManager().SetTimer(TimerHandle_Interact, this, &UWorldItemWidgetComponent::Interact, ItemComponent->InteractionTime, false);
			
			FTimerDelegate TimerDelegate;
			TimerDelegate.BindUObject(GetCurrentWorldItemWidgetComponent(), &UWorldItemWidgetComponent::Interact, Character);

			GetWorld()->GetTimerManager().SetTimer(TimerHandle_Interact, TimerDelegate, ItemComponent->InteractionTime, false);
		}
	}
}

void UZInteractComponent::EndInteract()
{
	if (!GetOwner()->HasAuthority())
	{
		ServerEndInteract();
	}

	InteractionData.bInteractHeld = false;

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interact);

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!Character) return;

	if (UWorldItemWidgetComponent* Interactable = GetCurrentWorldItemWidgetComponent())
	{
		Interactable->EndInteract(Character);
	}
}

void UZInteractComponent::Interact()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interact);

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!Character) return;

	if (UWorldItemWidgetComponent* Interactable = GetCurrentWorldItemWidgetComponent())
	{
		Interactable->Interact(Character);
	}
}

void UZInteractComponent::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

bool UZInteractComponent::ServerBeginInteract_Validate()
{
	return true;
}

void UZInteractComponent::ServerEndInteract_Implementation()
{
	EndInteract();
}

bool UZInteractComponent::ServerEndInteract_Validate()
{
	return true;
}

bool UZInteractComponent::IsInteracting() const
{
	return GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interact);
}

float UZInteractComponent::GetRemainingInteractionTime() const
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(TimerHandle_Interact);
}

void UZInteractComponent::PerformInteractionCheck()
{
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);

	AZ1Character* MyOwner = Cast<AZ1Character>(GetOwner());

	InteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyesLoc;
	FRotator EyesRot;
	MyOwner->GetActorEyesViewPoint(EyesLoc, EyesRot);

	FVector TraceStart = EyesLoc;
	TraceStart.X += 30;
	FVector TraceEnd = (EyesRot.Vector() * TraceDistance) + TraceStart;

	float Radius = 30.f;

	TArray<FHitResult> Hits;
	FCollisionShape Shape;
	Shape.SetSphere(Radius);

	bool bBlockingHit = GetWorld()->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectQueryParams, Shape);

	FColor LineColor = bBlockingHit ? FColor::Green : FColor::Red;
	for (FHitResult& Hit : Hits)
	{
		AZWorldItem* HitActor = Cast<AZWorldItem>(Hit.GetActor());
		if (IsValid(HitActor))
		{
			if (UWorldItemWidgetComponent* WorldItemComponent = Cast<UWorldItemWidgetComponent>(HitActor->GetComponentByClass(UWorldItemWidgetComponent::StaticClass())))
			{
				float Distance = (TraceStart - Hit.ImpactPoint).Size();

				if (WorldItemComponent != GetCurrentWorldItemWidgetComponent() && Distance <= WorldItemComponent->InteractionDistance)
				{
					if (CVarDebugDrawInteraction.GetValueOnGameThread())
					{
						DrawDebugSphere(GetWorld(), Hit.ImpactPoint, Radius, 32, LineColor, false, 2.0f);
					}
					FoundNewInteractable(WorldItemComponent, HitActor);
				}
				else if (Distance > WorldItemComponent->InteractionDistance && GetCurrentWorldItemWidgetComponent())
				{
					CouldntFindInteractable();
				}
				//return;
			}
			else
			{
				UE_LOG(LogZInteractionComp, Warning, TEXT("No UWorldItemWidgetComponent found on actor %s"), *HitActor->GetName());
			}
		}

	}
	if (CVarDebugDrawInteraction.GetValueOnGameThread())
	{
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, LineColor, false, 2.0f, 0, 2.0f);
	}

	//CouldntFindInteractable();
}

void UZInteractComponent::CouldntFindInteractable()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(TimerHandle_Interact))
	{
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_Interact);
	}

	if (UWorldItemWidgetComponent* Interactable = GetCurrentWorldItemWidgetComponent())
	{
		if (AZ1Character* Character = Cast<AZ1Character>(GetOwner()))
		{
			if (IsValid(Interactable))
			{
				Interactable->EndFocus(Character);
			}
		}

		if (InteractionData.bInteractHeld)
		{
			EndInteract();
		}
	}

	InteractionData.CurrentItemComponent = nullptr;
	InteractionData.CurrentWorldItem = nullptr;
}

void UZInteractComponent::FoundNewInteractable(UWorldItemWidgetComponent* WorldItemWidgetComponent, AZWorldItem* WorldItem)
{
	UE_LOG(LogZInteractionComp, Display, TEXT("We found and interactable"));

	EndInteract();

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	if (UWorldItemWidgetComponent* PrevInteractable = GetCurrentWorldItemWidgetComponent())
	{
		PrevInteractable->EndFocus(Character);
	}

	InteractionData.CurrentItemComponent = WorldItemWidgetComponent;
	InteractionData.CurrentWorldItem = WorldItem;

	WorldItemWidgetComponent->BeginFocus(Character);
}

bool UZInteractComponent::IsActorOutOfRange(AZWorldItem* WorldItem)
{
	if (!IsValid(WorldItem))
	{
		return false;
	}
	FVector EyeLocation;
	FRotator EyeRotation;
	AZ1Character* MyOwner = Cast<AZ1Character>(GetOwner());
	if (!IsValid(MyOwner))
	{
		return false;
	}
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	FVector FocusedActorLocation = WorldItem->GetActorLocation();
	float Distance = FVector::Dist(EyeLocation, FocusedActorLocation);
	return Distance <= TraceDistance;
}

bool UZInteractComponent::IsActorInSight(AZWorldItem* WorldItem)
{
	if (!IsValid(WorldItem))
	{
		return false;
	}

	// 1. 캐릭터의 시야 시작점(EyeLocation)과 방향(EyeRotation) 설정
	FVector EyeLocation;
	FRotator EyeRotation;
	AZ1Character* MyOwner = Cast<AZ1Character>(GetOwner());
	if (!IsValid(MyOwner)) 
	{
		return false;
	}

	// 2. FocusedActor와 캐릭터 시점 간의 방향 벡터 계산
	MyOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector ForwardVector = EyeRotation.Vector(); // 캐릭터의 시야 방향 벡터
	FVector ActorLocation = WorldItem->GetActorLocation();
	FVector ToActor = (ActorLocation - EyeLocation).GetSafeNormal(); // 방향 벡터 정규화

	// 3. 내적 계산: ForwardVector와 ToActor의 내적
	float DotProduct = FVector::DotProduct(ForwardVector, ToActor);

	// 4. 시야 각도 조건 설정
	float SightThreshold = FMath::Cos(FMath::DegreesToRadians(TraceRadius));
	// TraceRadius는 시야의 반각(도) - 예: 30도

	// 5. 내적 결과가 시야 각도 조건을 만족하는지 확인
	return DotProduct >= SightThreshold;
}


void UZInteractComponent::PrimaryInteract()
{
	if (!InteractionData.CurrentWorldItem)
	{
		FString DebugMsg = TEXT("No Focus Actor to Interact");
		//TODO : LOG
		return;
	}

	if (APawn* MyPawn = Cast<APawn>(GetOwner()))
	{
		IZInteractable::Execute_Interact(InteractionData.CurrentWorldItem, MyPawn);
	}
}