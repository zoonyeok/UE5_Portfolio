// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZTargetLockComponent.h"

#include "DrawDebugHelpers.h"
#include "Z1Character.h"
#include "AI/ZAICharacter.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

bool bDebugDraw = false;
static TAutoConsoleVariable<bool> CVarDebugDrawLockOn(
	TEXT("z.LockOnDebugDraw"),
	bDebugDraw,
	TEXT("Enable Debug Lines for LockOn Component."),
	ECVF_Default // Cheat
);

// Sets default values for this component's properties
UZTargetLockComponent::UZTargetLockComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
	// 기본값 설정
	LockRange = 1500.0f;
	LockAngle = 180.0f;
	LockedTarget = nullptr;
	InterpSpeed = 9.0f;

	CameraZValueScale = 100.f;
}

void UZTargetLockComponent::ToggleLockOnTarget()
{
	if (IsLocked())
	{
		ReleaseLock();
	}
	else
	{
		FindAndLockTarget();
	}
}

// Called when the game starts
void UZTargetLockComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UZTargetLockComponent::SetRotationMode(ERotationMode RotationMode)
{
	CurrentRotationMode = RotationMode;
	
	if (TObjectPtr<AZ1Character> Character = Cast<AZ1Character>(GetOwner()))
	{
		if (TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent = Character->GetCharacterMovement())
		{
			if (RotationMode == ERotationMode::FreeLook)
			{
				CharacterMovementComponent->bOrientRotationToMovement = true;
				CharacterMovementComponent->bUseControllerDesiredRotation = false;
			}
			if (RotationMode == ERotationMode::TargetLock)
			{
				CharacterMovementComponent->bOrientRotationToMovement = false;
				CharacterMovementComponent->bUseControllerDesiredRotation = true;
				//Character->bUseControllerRotationYaw = false;
			}
		}
	}
}


// Called every frame
void UZTargetLockComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	// 락온된 대상이 유효한지 지속적으로 확인 (예: 대상이 파괴되었거나 범위를 벗어났다면 해제)
	if (IsValid(LockedTarget))
	{
		if (!TargetAngleChecking(LockedTarget) || !TargetDistanceChecking(LockedTarget, LockRange))
		{
			ReleaseLock();
		}

		UpdateTargetingRotation(DeltaTime);
	}
	else
	{
		ReleaseLock();
	}
}

void UZTargetLockComponent::FindAndLockTarget()
{
	// 현재 소유자(플레이어)의 위치
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	FVector OwnerLocation = Owner->GetActorLocation();

	// 탐색 영역: 구형 영역으로 주변 Actor들을 검색
	TArray<FOverlapResult> OverlapResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(LockRange);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Owner);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults,
		OwnerLocation,
		FQuat::Identity,
		ECC_Pawn,
		Sphere,
		QueryParams
	);

	TArray<TObjectPtr<AActor>> CandidateTargets;
	if (bHit)
	{
		for (const FOverlapResult& Result : OverlapResults)
		{
			TObjectPtr<AActor> OverlappedActor = Result.GetActor();
			if (OverlappedActor && OverlappedActor->IsA<AZAICharacter>())
			{
				// 여기에 추가적인 필터 조건(예: 적 태그 체크, 팀 판별 등) 적용 가능
				CandidateTargets.Add(OverlappedActor);
			}
		}
	}

	// 가장 적절한 타겟 선택 (예: 가장 가까운 대상)
	TObjectPtr<AActor> BestTarget = SelectBestTarget(CandidateTargets);
	if (BestTarget)
	{
		LockedTarget = BestTarget;
		// 락온 시 추가 작업: UI 업데이트, 카메라 이동 등
		if (AZAICharacter* AICharacter = Cast<AZAICharacter>(LockedTarget))
		{
			AICharacter->ShowLockOnWidget(true);
		}

		bLocked = true;
		// TODO : Camera
		SetRotationMode(ERotationMode::TargetLock);
	}
}

void UZTargetLockComponent::ReleaseLock()
{
	// 락온 해제 시 추가 작업: UI 업데이트, 카메라 복귀 등
	if (AZAICharacter* AICharacter = Cast<AZAICharacter>(LockedTarget))
	{
		AICharacter->ShowLockOnWidget(false);
	}

	LockedTarget = nullptr;
	bLocked = false;
	// TODO : Camera
	SetRotationMode(ERotationMode::FreeLook);
}

bool UZTargetLockComponent::TargetDistanceChecking(const TObjectPtr<AActor>& SelectedActor, float BestDistance)
{
	TObjectPtr<AActor> Owner = GetOwner();
	FVector OwnerLocation = Owner->GetActorLocation();
	
	// 거리 점수 (가까울수록 좋은 타겟)
	float CurrentDistance = FVector::Dist(OwnerLocation, SelectedActor->GetActorLocation());
	
	if (CVarDebugDrawLockOn.GetValueOnGameThread())
	{
		// 타겟까지 선(Line) 그리기
		DrawDebugLine(
			GetWorld(),
			OwnerLocation,                        // 시작점 (플레이어 위치)
			LockedTarget->GetActorLocation(),        // 끝점 (타겟 위치)
			CurrentDistance < BestDistance ? FColor::Green : FColor::Red,                        // 색상
			false,                                 // 지속 여부 (false: 한 프레임만 표시)
			1.0f,                                  // 지속 시간
			0,                                     // 뎁스 우선순위
			2.0f                                   // 두께
		);
	}

	if (CurrentDistance > BestDistance)
	{
		return false;
	}

	return true;
}

bool UZTargetLockComponent::TargetAngleChecking(const TObjectPtr<AActor>& SelectedActor)
{
	TObjectPtr<AActor> Owner = GetOwner();
	FVector OwnerLocation = Owner->GetActorLocation();
	FVector OwnerForward = Owner->GetActorForwardVector();
	
	FVector DirToCandidate = (SelectedActor->GetActorLocation() - OwnerLocation).GetSafeNormal();
	float DotProduct = FVector::DotProduct(OwnerForward, DirToCandidate);
	float AngleDegrees = FMath::Acos(DotProduct) * (180.f / PI);

	if (CVarDebugDrawLockOn.GetValueOnGameThread())
	{
		// 시야각 원뿔 디버깅
		DrawDebugCone(
			GetWorld(),
			OwnerLocation,
			OwnerForward,
			LockRange,
			FMath::DegreesToRadians(LockAngle),
			FMath::DegreesToRadians(1),
			12,
			AngleDegrees > LockAngle ? FColor::Green : FColor::Red,  // 시야각 표시 (파란색)
			false,
			1.0f
		);
	}

	// 시야각 조건 검사
	if (AngleDegrees > LockAngle)
	{
		return false;
	}

	return true;
}

TObjectPtr<AActor> UZTargetLockComponent::SelectBestTarget(const TArray<TObjectPtr<AActor>>& TargetList)
{
	TObjectPtr<AActor> Owner = GetOwner();
	if (!Owner || TargetList.Num() == 0)
	{
		return nullptr;
	}

	TObjectPtr<AActor> BestTarget = nullptr;
	float BestScore = MAX_flt;
	
	for (TObjectPtr<AActor> Candidate : TargetList)
	{
		if (!Candidate || !Candidate->IsA<AZAICharacter>())
		{
			continue;
		}
		
		if (!TargetAngleChecking(Candidate))
		{
			continue;
		}

		if (CVarDebugDrawLockOn.GetValueOnGameThread())
		{
			DrawDebugPoint(
				GetWorld(),
				Candidate->GetActorLocation(),
				10.0f,        // 점 크기
				FColor::Red,  // 색상 (빨강)
				false,
				1.0f          // 지속 시간
			);
		}

		float Distance = FVector::Dist(Owner->GetActorLocation(), Candidate->GetActorLocation());
		if (TargetDistanceChecking(Candidate, BestScore))
		{
			BestScore = Distance;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

void UZTargetLockComponent::UpdateTargetingRotation(float DeltaTimeSeconds)
{
	AZ1Character* Character = Cast<AZ1Character>(GetOwner());
	if (!Character || !LockedTarget) return;

	AController* Controller = Character->GetController();
	if (!Controller) return;
	
	FVector CharacterLocation = Character->GetActorLocation();
	FVector TargetLocation = LockedTarget->GetActorLocation();
	TargetLocation.Z -= CameraZValueScale;

	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(CharacterLocation, TargetLocation);
	FRotator CurrentRotation = Character->GetActorRotation();

	FRotator InterpolatedRotation  = FMath::RInterpTo(CurrentRotation, TargetRotation, 	DeltaTimeSeconds, InterpSpeed);

	FRotator NewControlRotation(InterpolatedRotation .Pitch, InterpolatedRotation .Yaw,  Character->GetActorRotation().Roll);
	Controller->SetControlRotation(NewControlRotation);
}

