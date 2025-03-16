// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZWeaponTraceComponent.h"

#include "AI/ZAICharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/OverlapResult.h"
#include "Weapons/ZBaseWeapon.h"

// Sets default values for this component's properties
UZWeaponTraceComponent::UZWeaponTraceComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UZWeaponTraceComponent::GenerateCollisionMesh(const FVector& CurrentStart, const FVector& CurrentEnd)
{
	// const AActor* Owner = GetOwner();
	// const USkeletalMeshComponent* Mesh = Owner->FindComponentByClass<USkeletalMeshComponent>();
	// if (!Mesh) return;

	// 현재 프레임 위치 가져오기
	if (!bHasPreviousData)
	{
		PrevStartPos = CurrentStart;
		PrevEndPos = CurrentEnd;
		bHasPreviousData = true;
		return;
	}

	// 선형 보간을 통한 점 생성
	TArray<FVector> Points;
	for (int32 i = 0; i <= SegmentCount; ++i)
	{
		const float Alpha = static_cast<float>(i) / SegmentCount;

		// 이전 프레임 보간
		const FVector PrevPoint = FMath::Lerp(PrevStartPos, PrevEndPos, Alpha);
		// 현재 프레임 보간
		const FVector CurrentPoint = FMath::Lerp(CurrentStart, CurrentEnd, Alpha);

		Points.Add(PrevPoint);
		Points.Add(CurrentPoint);
	}

	// 삼각형 생성 및 충돌 검사
	for (int32 i = 0; i < Points.Num() - 2; ++i)
	{
		const FVector& A = Points[i];
		const FVector& B = Points[i + 1];
		const FVector& C = Points[i + 2];

		CheckTriangleCollision(A, B, C);
	}

	// 이전 데이터 업데이트
	PrevStartPos = CurrentStart;
	PrevEndPos = CurrentEnd;
}

void UZWeaponTraceComponent::CheckTriangleCollision(const FVector& A, const FVector& B, const FVector& C)
{
	UWorld* World = GetWorld();
	if (!World) return;

	// 삼각형 내부 점 샘플링
	const TArray<FVector> SamplePoints = {
		A, B, C,
		(A + B) / 2,
		(B + C) / 2,
		(C + A) / 2
	};

	// 히트박스 검출 파라미터
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	for (const FVector& Point : SamplePoints)
	{
		TArray<FOverlapResult> Overlaps;
		if (World->OverlapMultiByChannel(
			Overlaps,
			Point,
			FQuat::Identity,
			ECC_GameTraceChannel2, // 히트박스 채널
			FCollisionShape::MakeSphere(15.0f), // 검사 반경
			Params))
		{
			// 충돌 처리 로직
			if (IsTriangleIntersectingWithHitbox(Overlaps))
			{
				// Weapon Damage Logic
			}
		}
	}

	// 디버그 드로잉
	if (bDebugDraw)
	{
		DrawDebugLine(World, A, B, FColor::Blue, false, 1.0f);
		DrawDebugLine(World, B, C, FColor::Blue, false, 1.0f);
		DrawDebugLine(World, C, A, FColor::Blue, false, 1.0f);
	}
}

bool UZWeaponTraceComponent::IsTriangleIntersectingWithHitbox(const TArray<FOverlapResult>& OverlapResults)
{
	for (const auto& Overlap : OverlapResults)
	{
		TObjectPtr<AZAICharacter> AICharacter = Cast<AZAICharacter>(Overlap.GetActor());
		if (IsValid(AICharacter))
		{
			TObjectPtr<UCapsuleComponent> HitboxComponent = AICharacter->FindComponentByClass<UCapsuleComponent>();
			if (IsValid(HitboxComponent))
			{
				return true;
			}
		}
	}
	return false;
}