// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZWeaponTraceComponent.generated.h"


/*
 * 무기 궤적을 기반으로 충돌 검사 및 데미지 처리 수행
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZWeaponTraceComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZWeaponTraceComponent();

	void GenerateCollisionMesh(const FVector& CurrentStart, const FVector& CurrentEnd);
protected:
	// 분할 개수 설정
	UPROPERTY(EditDefaultsOnly, Category="Weapon", meta=(ClampMin=2))
	int32 SegmentCount = 5;

	UPROPERTY(EditAnywhere)
	bool bDebugDraw = true;
	
	void CheckTriangleCollision(const FVector& A, const FVector& B, const FVector& C);
	bool IsTriangleIntersectingWithHitbox(const TArray<FOverlapResult>& Triangle);


private:
	// 이전 프레임 위치 저장
	FVector PrevStartPos;
	FVector PrevEndPos;
	bool bHasPreviousData = false;
		
};
