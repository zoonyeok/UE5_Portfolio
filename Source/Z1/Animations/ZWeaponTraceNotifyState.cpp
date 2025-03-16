// Fill out your copyright notice in the Description page of Project Settings.

#include "Animations/ZWeaponTraceNotifyState.h"
#include "Z1Character.h"
#include "AI/ZAICharacter.h"
#include "Animations/UZAnimUtils.h"
#include "Components/ZAttributeComponent.h"
#include "Engine/OverlapResult.h"

DEFINE_LOG_CATEGORY(LogWeaponTrace);

constexpr int32 DEBUG_SOCKET_LINE    = 1 << 0; // 0001 (1)
constexpr int32 DEBUG_COLLISION      = 1 << 1; // 0010 (2)

static TAutoConsoleVariable<int32> CVarDebugDrawWeaponTrace(
	TEXT("z.WeaponTraceDebugDraw"),
	0,
	TEXT("Enable Debug WeaponTrace for ComboAction.\n1: Socket + LINE, 2: Collision"),
	ECVF_Default // Cheat
);

void UZWeaponTraceNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	CurrentAnimMontage = Cast<UAnimMontage>(Animation);

	if (MeshComp)
	{
		Owner = Cast<AZ1Character>(MeshComp->GetOwner());
		if (!IsValid(Owner)) return;

		WeaponBoneName = Owner->GetWeaponSocketParentBoneName();
		// TObjectPtr<UZInventoryItem> CurrentWeapon = Owner->GetCurrentWeapon(EItemSlotType::EIS_Weapon_1);
		//
		// if (UZAttributeComponent* AttributeComponent = Owner->GetComponentByClass<UZAttributeComponent>())
		// {
		// 	
		// }

		ClearData();
	}
}

void UZWeaponTraceNotifyState::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                          float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!Owner)
	{
		Owner = Cast<AZ1Character>(MeshComp->GetOwner());

		if (!IsValid(Owner)) return;
	}
	
	TPair<FTransform, FTransform> CurrentTransform = Owner->GetWeaponSocketTransform();

	if (UWorld* InWorld = Owner->GetWorld())
	{
		// UE_LOG(LogWeaponTrace, Display, TEXT("WeaponTransform Start : %s"), *CurrentTransform.Key.ToString());
		// UE_LOG(LogWeaponTrace, Display, TEXT("WeaponTransform End : %s"), *CurrentTransform.Value.ToString());
			
		if (CVarDebugDrawWeaponTrace.GetValueOnGameThread() == DEBUG_SOCKET_LINE)
		{
			DrawDebugSphere(InWorld, CurrentTransform.Key.GetTranslation(), Radius, AnimSegmentCount, FColor::Red,false, LifeTime);
			DrawDebugSphere(InWorld, CurrentTransform.Value.GetTranslation(), Radius, AnimSegmentCount, FColor::Red,false, LifeTime);
		}
	}

	// 무기 궤적을 보간하여 충돌 감지
	GenerateCollisionMesh(CurrentTransform);
}

void UZWeaponTraceNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                         const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	ClearData();
}

void UZWeaponTraceNotifyState::GenerateCollisionMesh(const TPair<FTransform, FTransform>& CurrentTransform)
{
	// 아직 이전 프레임 데이터가 없으면, 이번 프레임 정보를 저장하고 종료
	if (!bHasPreviousData)
	{
		PrevTransform = CurrentTransform;
		bHasPreviousData = true;
		return;
	}

	TArray<FTransform> TransformPoints;
	// 0 ~ SegmentCount까지, 일정 Alpha 간격으로 보간
	for (int32 i = 0; i <= SegmentCount; ++i)
	{
		float Alpha = static_cast<float>(i) / SegmentCount;

		// "이전 프레임" 구간 보간
		FTransform PrevTF = UUZAnimUtils::LerpTransform(PrevTransform.Key, PrevTransform.Value, Alpha);

		// "현재 프레임" 구간 보간
		FTransform CurrTF = UUZAnimUtils::LerpTransform(CurrentTransform.Key, CurrentTransform.Value, Alpha);

		// 보간된 두 Transform을 모아서 배열에 저장
		TransformPoints.Add(PrevTF);
		TransformPoints.Add(CurrTF);
	}

	// 이제 TransformPoints에는 (SegmentCount+1)*2 개의 Transform이 들어있음
	// 연속된 3개 Transform의 Location을 삼각형으로 연결해 충돌 검사
	// (본 예시는 “2개씩” 추가하므로 꼭 -2 까지 처리해야 함)

	for (int32 i = 0; i < TransformPoints.Num() - 2; ++i)
	{
		const FVector A = TransformPoints[i].GetLocation();
		const FVector B = TransformPoints[i + 1].GetLocation();
		const FVector C = TransformPoints[i + 2].GetLocation();
		
		CheckTriangleCollision(A, B, C);
	}

	// 이번 프레임 것을 다음 프레임에서 "이전"으로 활용
	PrevTransform = CurrentTransform;
}

void UZWeaponTraceNotifyState::CheckTriangleCollision(const FVector& A, const FVector& B, const FVector& C)
{
	UWorld* World = Owner->GetWorld();
	if (!World) return;

	// 삼각형 내부/중심을 여러 점으로 샘플링하여 Overlap 검사(OverlapMultiByChannel)
	const TArray<FVector> SamplePoints = {
		A, B, C,
		(A + B) * 0.5f,
		(B + C) * 0.5f,
		(C + A) * 0.5f
	};

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Owner);

	for (const FVector& Point : SamplePoints)
	{
		TArray<FOverlapResult> Overlaps;
		bool bOverlap = World->OverlapMultiByChannel(
			Overlaps,
			Point,
			FQuat::Identity,
			CollisionChannel,
			FCollisionShape::MakeSphere(5.0f), // 🎯 작은 구체를 사용하여 라인 효과 구현
			Params
		);

		if (bOverlap)
		{
			TriangleIntersectingChecking(Overlaps);
		}

		if (CVarDebugDrawWeaponTrace.GetValueOnGameThread() == DEBUG_COLLISION)
		{
			DrawDebugSphere(World, Point, 5.0f, 12, FColor::Blue, false, 1.0f);
		}
	}

	if (CVarDebugDrawWeaponTrace.GetValueOnGameThread() == DEBUG_SOCKET_LINE)
	{
		DrawDebugLine(World, A, B, FColor::Blue, false, 1.0f);
		DrawDebugLine(World, B, C, FColor::Blue, false, 1.0f);
		DrawDebugLine(World, C, A, FColor::Blue, false, 1.0f);
	}
}

void UZWeaponTraceNotifyState::TriangleIntersectingChecking(const TArray<FOverlapResult>& OverlapResults)
{
	for (const auto& Overlap : OverlapResults)
	{
		if (AlreadyHitActors.Find(Overlap.GetActor()))
		{
			continue;
		}
		
		if (TObjectPtr<AZAICharacter> Dst = Cast<AZAICharacter>(Overlap.GetActor()))
		{
			if (UZAttributeComponent* AttributeComponent = Dst->GetComponentByClass<UZAttributeComponent>())
			{
				AttributeComponent->ChangeCurrentHP(Owner, -10);
				AlreadyHitActors.Add((Overlap.GetActor()));

				static int32 CheckCount = 0;
				UE_LOG(LogWeaponTrace, Display, TEXT("Check Triangle Collision Count : %d"), CheckCount++);
			}
		}
	}
}

void UZWeaponTraceNotifyState::ClearData()
{
	bHasPreviousData = false;
	AlreadyHitActors.Empty();
}
