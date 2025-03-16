// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/WidgetComponent.h"
#include "ZTargetLockComponent.generated.h"

UENUM(BlueprintType)
enum class ERotationMode : uint8
{
	TargetLock UMETA(DisplayName = "Target Lock"), // Orient to Camera
	FreeLook UMETA(DisplayName = "Free Look")	  // Orient to Movement
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZTargetLockComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UZTargetLockComponent();
	
	void ToggleLockOnTarget();
	
	FORCEINLINE bool IsLocked() const { return bLocked; }

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 락온 수행 함수
	UFUNCTION(BlueprintCallable, Category = "Target Lock")
	void FindAndLockTarget();

	// 락온 해제 함수
	UFUNCTION(BlueprintCallable, Category = "Target Lock")
	void ReleaseLock();
	
	// 내부: 조건에 맞는 타겟을 선택하는 함수
	TObjectPtr<AActor> SelectBestTarget(const TArray<TObjectPtr<AActor>>& TargetList);
	
	// 현재 락온된 대상
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<AActor> LockedTarget;
	
	// 락온 범위 (예: 1500.0f)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock", meta = (AllowPrivateAccess = "true"))
	float LockRange;

	// 락온 허용 시야각 (플레이어의 전방 기준, 예: 45도)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock", meta = (AllowPrivateAccess = "true"))
	float LockAngle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock", meta = (AllowPrivateAccess = "true"))
	float InterpSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Target Lock", meta = (AllowPrivateAccess = "true"))
	float CameraZValueScale;
	
	bool bLocked = false;

	UPROPERTY()
	ERotationMode CurrentRotationMode = ERotationMode::FreeLook;

	void UpdateTargetingRotation(float DeltaTimeSeconds);
	void SetRotationMode(ERotationMode RotationMode);
	
	bool TargetDistanceChecking(const TObjectPtr<AActor>& SelectedActor, float BestDistance);
	bool TargetAngleChecking(const TObjectPtr<AActor>& SelectedActor);
};

