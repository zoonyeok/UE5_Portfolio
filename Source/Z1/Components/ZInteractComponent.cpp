// Fill out your copyright notice in the Description page of Project Settings.

#include "ZInteractComponent.h"

#include "DrawDebugHelpers.h"
#include "GameFramework/GameStateBase.h"
#include "Interfaces/ZInteractable.h"
#include "Interfaces/ZPickable.h"
#include "Items/ZWorldItem.h"
#include "Net/UnrealNetwork.h"
#include "TimerManager.h"
#include "UI/WorldItemWidgetComponent.h"
#include "Z1Character.h"

DEFINE_LOG_CATEGORY_STATIC(LogZInteractionComp, Log, All);

static TAutoConsoleVariable<bool>
CVarDebugDrawInteraction(TEXT("z.InteractionDebugDraw"), false,
                         TEXT("Draw interaction traces."), ECVF_Cheat);

UZInteractComponent::UZInteractComponent()
{
	/*
	 * 컴포넌트 RPC와 InteractionState 복제를 위해 필요합니다.
	 * 소유 Character도 bReplicates가 true여야 합니다.
	 */
	SetIsReplicatedByDefault(true);

	PrimaryComponentTick.bCanEverTick = true;
}

void UZInteractComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UZInteractComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(UZInteractComponent, InteractionState,COND_OwnerOnly);
}

void UZInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType,FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	/*
	 * 상호작용 탐색은 소유 클라이언트에서만 수행합니다.
	 *
	 * Dedicated Server:
	 *   플레이어 Pawn은 로컬 컨트롤이 아니므로 실행하지 않습니다.
	 *
	 * Client:
	 *   자신의 Pawn만 실행합니다.
	 *
	 * Listen Server:
	 *   호스트의 로컬 Pawn은 실행합니다.
	 */
	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!IsValid(OwnerPawn) || !OwnerPawn->IsLocallyControlled())
	{
		return;
	}

	AZWorldItem* CurrentWorldItem = GetCurrentWorldItem();
	if (IsValid(CurrentWorldItem))
	{
		if (IsActorOutOfRange(CurrentWorldItem) || !IsActorInSight(CurrentWorldItem))
		{
			CouldntFindInteractable();
		}
	}

	/*
	 * 버튼을 누르고 있는 동안에는 새로운 대상을 찾지 않습니다.
	 */
	if (LocalInteractionData.bInteractHeld)
	{
		return;
	}

	const float TimeSinceLastCheck = GetWorld()->TimeSince(LocalInteractionData.LastInteractionCheckTime);
	if (TimeSinceLastCheck >= InteractionCheckFrequency)
	{
		PerformInteractionCheck();
	}
}

void UZInteractComponent::PerformInteractionCheck()
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!IsValid(Character) || !Character->IsLocallyControlled())
	{
		return;
	}

	LocalInteractionData.LastInteractionCheckTime = GetWorld()->GetTimeSeconds();

	FVector EyeLocation;
	FRotator EyeRotation;

	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector TraceStart = EyeLocation;
	const FVector TraceEnd = TraceStart + EyeRotation.Vector() * TraceDistance;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(CollisionChannel);
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(LocalInteractionSweep),false, Character);

	FCollisionShape CollisionShape;
	CollisionShape.SetSphere(TraceRadius);

	TArray<FHitResult> Hits;

	const bool bAnyHit = GetWorld()->SweepMultiByObjectType(Hits, TraceStart, TraceEnd, FQuat::Identity, ObjectQueryParams, CollisionShape, QueryParams);

	AZWorldItem* BestWorldItem = nullptr;
	UWorldItemWidgetComponent* BestItemComponent = nullptr;

	float BestDistanceSquared = TNumericLimits<float>::Max();

	for (const FHitResult& Hit : Hits)
	{
		AZWorldItem* HitWorldItem = Cast<AZWorldItem>(Hit.GetActor());

		if (!IsValid(HitWorldItem))
		{
			continue;
		}

		/*
		 * 상호작용 또는 획득 인터페이스 중 하나는
		 * 구현하고 있어야 합니다.
		 */
		if (!HitWorldItem->Implements<UZInteractable>() && !HitWorldItem->Implements<UZPickable>())
		{
			continue;
		}

		UWorldItemWidgetComponent* ItemComponent =
			HitWorldItem->FindComponentByClass<UWorldItemWidgetComponent>();

		if (!IsValid(ItemComponent))
		{
			UE_LOG(LogZInteractionComp, Warning, TEXT("No UWorldItemWidgetComponent on %s"), *HitWorldItem->GetName());
			continue;
		}
		
		const FVector TargetPoint = Hit.ImpactPoint.IsNearlyZero() ? HitWorldItem->GetActorLocation() : static_cast<FVector>(Hit.ImpactPoint);

		const float DistanceSquared = FVector::DistSquared(TraceStart, TargetPoint);

		const float AllowedDistance = FMath::Min(TraceDistance, ItemComponent->InteractionDistance);

		if (DistanceSquared > FMath::Square(AllowedDistance))
		{
			continue;
		}

		if (DistanceSquared < BestDistanceSquared)
		{
			BestDistanceSquared = DistanceSquared;
			BestWorldItem = HitWorldItem;
			BestItemComponent = ItemComponent;
		}
	}

	if (IsValid(BestWorldItem) && IsValid(BestItemComponent))
	{
		if (BestWorldItem != GetCurrentWorldItem())
		{
			FoundNewInteractable(BestItemComponent, BestWorldItem);
		}
	}
	else if (IsValid(GetCurrentWorldItem()))
	{
		CouldntFindInteractable();
	}

	DrawDebugInfo(TraceStart, TraceEnd, Hits, bAnyHit);
}

void UZInteractComponent::FoundNewInteractable(
	UWorldItemWidgetComponent* WorldItemWidgetComponent,
	AZWorldItem* WorldItem)
{
	if (!IsValid(WorldItemWidgetComponent) || !IsValid(WorldItem))
	{
		return;
	}

	if (WorldItem == GetCurrentWorldItem())
	{
		return;
	}

	if (LocalInteractionData.bInteractHeld)
	{
		EndInteract();
	}

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());

	if (!IsValid(Character))
	{
		return;
	}

	UWorldItemWidgetComponent* PreviousComponent =
		GetCurrentWorldItemWidgetComponent();

	if (IsValid(PreviousComponent))
	{
		PreviousComponent->EndFocus(Character);
	}

	LocalInteractionData.CurrentItemComponent = WorldItemWidgetComponent;

	LocalInteractionData.CurrentWorldItem = WorldItem;

	WorldItemWidgetComponent->BeginFocus(Character);

	UE_LOG(LogZInteractionComp, Verbose, TEXT("Found interactable: %s"),
	       *WorldItem->GetName());
}

void UZInteractComponent::CouldntFindInteractable()
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());

	UWorldItemWidgetComponent* ItemComponent =
		GetCurrentWorldItemWidgetComponent();

	if (LocalInteractionData.bInteractHeld)
	{
		EndInteract();
	}

	if (IsValid(Character) && IsValid(ItemComponent))
	{
		ItemComponent->EndFocus(Character);
	}

	LocalInteractionData.CurrentItemComponent = nullptr;
	LocalInteractionData.CurrentWorldItem = nullptr;
	LocalInteractionData.bInteractHeld = false;
}

void UZInteractComponent::BeginInteract()
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!IsValid(Character) || !Character->IsLocallyControlled())
	{
		return;
	}

	/*
	 * Enhanced Input의 Triggered 등에 잘못 연결됐을 경우
	 * 프레임마다 요청하는 것을 방지합니다.
	 */
	if (LocalInteractionData.bInteractHeld)
	{
		return;
	}

	AZWorldItem* CandidateTarget = GetCurrentWorldItem();
	if (!IsValid(CandidateTarget))
	{
		return;
	}

	LocalInteractionData.bInteractHeld = true;

	/*
	 * 즉각적인 조작감을 위한 로컬 UI 예측입니다.
	 * 여기서는 게임 상태를 변경하면 안 됩니다.
	 */
	StartLocalInteractionFeedback();

	if (Character->HasAuthority())
	{
		/*
		 * Standalone 또는 Listen Server의 로컬 플레이어
		 */
		if (ValidateInteractionTarget(CandidateTarget))
		{
			BeginInteractionAuthority(CandidateTarget);
		}
		else
		{
			ClientInteractionRejected_Implementation();
		}
	}
	else
	{
		/*
		 * 일반 네트워크 클라이언트
		 */
		ServerBeginInteract(CandidateTarget);
	}
}

void UZInteractComponent::EndInteract()
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());

	if (!IsValid(Character) || !Character->IsLocallyControlled())
	{
		return;
	}

	const bool bHadLocalInteraction = LocalInteractionData.bInteractHeld;

	LocalInteractionData.bInteractHeld = false;

	StopLocalInteractionFeedback();

	/*
	 * 시작한 상호작용이 없다면 취소 RPC도 보내지 않습니다.
	 */
	if (!bHadLocalInteraction && !InteractionState.bIsInteracting)
	{
		return;
	}

	if (Character->HasAuthority())
	{
		CancelInteractionAuthority();
	}
	else
	{
		ServerEndInteract();
	}
}

void UZInteractComponent::PrimaryInteract()
{
	BeginInteract();
}

void UZInteractComponent::ServerBeginInteract_Implementation(
	AZWorldItem* CandidateTarget)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	const float CurrentTime = GetWorld()->GetTimeSeconds();

	/*
	 * 순간 상호작용 RPC 연타 방지
	 */
	if (CurrentTime - LastServerInteractionRequestTime < MinServerInteractionInterval)
	{
		ClientInteractionRejected();
		return;
	}

	LastServerInteractionRequestTime = CurrentTime;

	if (InteractionState.bIsInteracting)
	{
		ClientInteractionRejected();
		return;
	}

	if (!ValidateInteractionTarget(CandidateTarget))
	{
		ClientInteractionRejected();
		return;
	}

	BeginInteractionAuthority(CandidateTarget);
}

void UZInteractComponent::ServerEndInteract_Implementation()
{
	CancelInteractionAuthority();
}

void UZInteractComponent::ClientInteractionRejected_Implementation()
{
	LocalInteractionData.bInteractHeld = false;

	StopLocalInteractionFeedback();
}

void UZInteractComponent::BeginInteractionAuthority(
	AZWorldItem* CandidateTarget)
{
	if (!GetOwner()->HasAuthority() || !IsValid(CandidateTarget))
	{
		return;
	}

	if (InteractionState.bIsInteracting)
	{
		CancelInteractionAuthority();
	}

	const float InteractionDuration = GetInteractionDuration(CandidateTarget);

	InteractionState.ActiveWorldItem = CandidateTarget;

	InteractionState.bIsInteracting = true;

	InteractionState.InteractionEndServerTime =
		GetWorld()->GetTimeSeconds() + InteractionDuration;

	/*
	 * 즉시 상호작용
	 */
	if (InteractionDuration <= KINDA_SMALL_NUMBER)
	{
		CompleteInteractionAuthority();
		return;
	}

	/*
	 * 실제 완료 타이머는 서버에만 생성합니다.
	 */
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_ServerInteraction, this,
		&ThisClass::CompleteInteractionAuthority, InteractionDuration, false);

	GetOwner()->ForceNetUpdate();//?
}

void UZInteractComponent::CompleteInteractionAuthority()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ServerInteraction);

	AZWorldItem* Target = InteractionState.ActiveWorldItem.Get();

	/*
	 * 홀드 도중 플레이어가 이동하거나
	 * 다른 플레이어가 먼저 획득했을 수 있으므로 재검증합니다.
	 */
	if (!ValidateInteractionTarget(Target))
	{
		CancelInteractionAuthority();
		return;
	}

	AZ1Character* Character = Cast<AZ1Character>(GetOwner());

	if (!IsValid(Character))
	{
		CancelInteractionAuthority();
		return;
	}

	/*
	 * 여기부터 실제 게임 상태를 변경하는 서버 권위 구간입니다.
	 */
	if (Target->Implements<UZInteractable>())
	{
		IZInteractable::Execute_Interact(Target, Character);
	}

	/*
	 * 기존 코드의 Both 동작을 유지하기 위해
	 * 두 인터페이스를 모두 실행합니다.
	 *
	 * 둘 중 하나만 실행해야 한다면 else if로 변경하거나,
	 * 별도의 EInteractionType을 서버에서 결정하십시오.
	 */
	if (IsValid(Target) && Target->Implements<UZPickable>())
	{
		IZPickable::Execute_PickUp(Target, Character);
	}

	InteractionState.Reset();

	GetOwner()->ForceNetUpdate();
}

void UZInteractComponent::CancelInteractionAuthority()
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_ServerInteraction);

	InteractionState.Reset();

	GetOwner()->ForceNetUpdate();
}

bool UZInteractComponent::ValidateInteractionTarget(AZWorldItem* CandidateTarget) const
{
	/*
	 * 이 함수는 서버에서만 최종 판정에 사용합니다.
	 */
	if (!GetOwner()->HasAuthority())
	{
		return false;
	}

	const AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!IsValid(Character) || !IsValid(CandidateTarget))
	{
		return false;
	}

	if (!CandidateTarget->Implements<UZInteractable>() && !CandidateTarget->Implements<UZPickable>())
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector TargetLocation = CandidateTarget->GetActorLocation();

	float AllowedDistance = TraceDistance;

	const UWorldItemWidgetComponent* ItemComponent = CandidateTarget->FindComponentByClass<UWorldItemWidgetComponent>();

	if (IsValid(ItemComponent))
	{
		AllowedDistance = FMath::Min(TraceDistance, ItemComponent->InteractionDistance);
	}

	/*
	 * 네트워크 위치 오차에 대한 작은 허용치입니다.
	 */
	constexpr float NetworkTolerance = 50.f;

	AllowedDistance += NetworkTolerance;

	if (FVector::DistSquared(EyeLocation, TargetLocation) > FMath::Square(AllowedDistance))
	{
		return false;
	}

	/*
	 * 서버가 알고 있는 시야 방향으로 각도를 검증합니다.
	 */
	const FVector ToTarget = (TargetLocation - EyeLocation).GetSafeNormal();

	const float ViewDot = FVector::DotProduct(EyeRotation.Vector(), ToTarget);

	const float MinimumViewDot = FMath::Cos(FMath::DegreesToRadians(MaxInteractionAngle));

	if (ViewDot < MinimumViewDot)
	{
		return false;
	}

	/*
	 * 벽이나 장애물에 가려졌는지 검증합니다.
	 */
	FHitResult HitResult;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ServerInteractionTrace),false, Character);
	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, EyeLocation, TargetLocation, ECC_Visibility, QueryParams);
	if (bHit && HitResult.GetActor() != CandidateTarget)
	{
		return false;
	}

	/*
	 * 추가할 서버 검증:
	 *
	 * - 아이템이 이미 획득됐는가?
	 * - 문이나 상자가 잠겼는가?
	 * - 캐릭터가 사망·경직 상태인가?
	 * - 인벤토리에 공간이 있는가?
	 * - 쿨다운 중인가?
	 */

	return true;
}

float UZInteractComponent::GetInteractionDuration(AZWorldItem* Target) const
{
	if (!IsValid(Target))
	{
		return 0.f;
	}

	const UWorldItemWidgetComponent* ItemComponent = Target->FindComponentByClass<UWorldItemWidgetComponent>();
	if (!IsValid(ItemComponent))
	{
		return 0.f;
	}

	return FMath::Max(0.f, ItemComponent->InteractionTime);
}

void UZInteractComponent::StartLocalInteractionFeedback()
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	UWorldItemWidgetComponent* ItemComponent = GetCurrentWorldItemWidgetComponent();
	if (!IsValid(Character) || !IsValid(ItemComponent))
	{
		return;
	}

	/*
	 * 이 함수가 위젯 진행 표시만 수행하도록 해야 합니다.
	 * 아이템 획득이나 문 열기 등을 실행하면 안 됩니다.
	 */
	ItemComponent->BeginInteract(Character);
}

void UZInteractComponent::StopLocalInteractionFeedback()
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	UWorldItemWidgetComponent* ItemComponent = GetCurrentWorldItemWidgetComponent();
	if (!IsValid(Character) || !IsValid(ItemComponent))
	{
		return;
	}

	/*
	 * UI 진행 표시 종료만 수행해야 합니다.
	 */
	ItemComponent->EndInteract(Character);
}

void UZInteractComponent::OnRep_InteractionState()
{
	if (InteractionState.bIsInteracting)
	{
		/*
		 * 서버가 상호작용 시작을 승인했습니다.
		 *
		 * 로컬 예측으로 이미 UI를 시작했으므로
		 * 여기서는 진행 종료 시간을 동기화하면 됩니다.
		 */
		return;
	}

	/*
	 * 서버에서 완료 또는 취소되었습니다.
	 */
	LocalInteractionData.bInteractHeld = false;

	StopLocalInteractionFeedback();
}

bool UZInteractComponent::IsInteracting() const
{
	/*
	 * 로컬 예측 상태 또는 서버 승인 상태 중
	 * 하나라도 활성화되어 있으면 true입니다.
	 */
	return LocalInteractionData.bInteractHeld || InteractionState.bIsInteracting;
}

float UZInteractComponent::GetRemainingInteractionTime() const
{
	if (!InteractionState.bIsInteracting)
	{
		return 0.f;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	
	const float CurrentServerTime = IsValid(GameState) ? GameState->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();

	return FMath::Max(0.f, InteractionState.InteractionEndServerTime - CurrentServerTime);
}

bool UZInteractComponent::IsActorOutOfRange(const AZWorldItem* WorldItem) const
{
	const AZ1Character* Character = Cast<AZ1Character>(GetOwner());

	if (!IsValid(Character) || !IsValid(WorldItem))
	{
		return true;
	}

	FVector EyeLocation;
	FRotator EyeRotation;

	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	float AllowedDistance = TraceDistance;

	const UWorldItemWidgetComponent* ItemComponent =
		WorldItem->FindComponentByClass<UWorldItemWidgetComponent>();

	if (IsValid(ItemComponent))
	{
		AllowedDistance =
			FMath::Min(TraceDistance, ItemComponent->InteractionDistance);
	}

	return FVector::DistSquared(EyeLocation, WorldItem->GetActorLocation()) >
		FMath::Square(AllowedDistance);
}

bool UZInteractComponent::IsActorInSight(const AZWorldItem* WorldItem) const
{
	const AZ1Character* Character = Cast<AZ1Character>(GetOwner());

	if (!IsValid(Character) || !IsValid(WorldItem))
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	Character->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FVector ToActor = (WorldItem->GetActorLocation() - EyeLocation).GetSafeNormal();

	const float DotProduct = FVector::DotProduct(EyeRotation.Vector(), ToActor);

	const float MinimumDotProduct = FMath::Cos(FMath::DegreesToRadians(MaxInteractionAngle));

	return DotProduct >= MinimumDotProduct;
}

void UZInteractComponent::DrawDebugInfo(const FVector& Start, const FVector& End, const TArray<FHitResult>& Hits, bool bAnyHit) const
{
	if (!CVarDebugDrawInteraction.GetValueOnGameThread())
	{
		return;
	}

	const FColor DebugColor = bAnyHit ? FColor::Green : FColor::Red;

	for (const FHitResult& Hit : Hits)
	{
		DrawDebugSphere(GetWorld(), Hit.ImpactPoint, TraceRadius, 16, DebugColor,
		                false, InteractionCheckFrequency);
	}

	DrawDebugLine(GetWorld(), Start, End, DebugColor, false,
	              InteractionCheckFrequency, 0, 1.f);
}
