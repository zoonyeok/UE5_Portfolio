// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ZInteractComponent.generated.h"

class AZWorldItem;
class UWorldItemWidgetComponent;

/**
 * 소유 클라이언트에서만 사용하는 탐색·UI 상태입니다.
 * 서버로 복제하지 않습니다.
 */
USTRUCT()
struct FLocalInteractionData
{
	GENERATED_BODY()

	FLocalInteractionData()
		: CurrentItemComponent(nullptr)
		, CurrentWorldItem(nullptr)
		, LastInteractionCheckTime(0.f)
		, bInteractHeld(false)
	{
	}

	UPROPERTY(Transient)
	TObjectPtr<UWorldItemWidgetComponent> CurrentItemComponent;

	UPROPERTY(Transient)
	TObjectPtr<AZWorldItem> CurrentWorldItem;

	float LastInteractionCheckTime;
	bool bInteractHeld;
};

/**
 * 서버에서 관리하고 소유 클라이언트에 복제하는 상태입니다.
 */
USTRUCT()
struct FReplicatedInteractionState
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<AZWorldItem> ActiveWorldItem = nullptr;

	/**
	 * 클라이언트에서 GameState의 서버 시간과 비교하여
	 * 남은 상호작용 시간을 계산합니다.
	 */
	UPROPERTY()
	float InteractionEndServerTime = 0.f;

	UPROPERTY()
	bool bIsInteracting = false;

	void Reset()
	{
		ActiveWorldItem = nullptr;
		InteractionEndServerTime = 0.f;
		bIsInteracting = false;
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class Z1_API UZInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZInteractComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 입력 Press에서 호출합니다. */
	void BeginInteract();

	/** 입력 Release 또는 Cancel에서 호출합니다. */
	void EndInteract();

	/**
	 * 기존 호출부 호환용입니다.
	 * 실질적으로 BeginInteract()를 호출합니다.
	 */
	void PrimaryInteract();

	bool IsInteracting() const;
	float GetRemainingInteractionTime() const;

protected:
	/*
	 * 로컬 클라이언트 전용
	 */
	UPROPERTY(Transient)
	FLocalInteractionData LocalInteractionData;

	/*
	 * 서버 권위 상태
	 */
	UPROPERTY(ReplicatedUsing = OnRep_InteractionState)
	FReplicatedInteractionState InteractionState;

	UFUNCTION()
	void OnRep_InteractionState();

	/*
	 * 클라이언트 → 서버
	 *
	 * 클라이언트가 선택한 CandidateTarget은 신뢰하지 않고
	 * 서버에서 반드시 다시 검증합니다.
	 */
	UFUNCTION(Server, Reliable)
	void ServerBeginInteract(AZWorldItem* CandidateTarget);

	UFUNCTION(Server, Reliable)
	void ServerEndInteract();

	/*
	 * 서버가 상호작용 요청을 거절했을 때
	 * 클라이언트의 예측 UI를 정리합니다.
	 */
	UFUNCTION(Client, Reliable)
	void ClientInteractionRejected();

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Trace", meta = (ClampMin = "0.01"))
	float InteractionCheckFrequency = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Trace", meta = (ClampMin = "0.0"))
	float TraceDistance = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Trace", meta = (ClampMin = "0.0"))
	float TraceRadius = 30.f;

	/** TraceRadius와 분리된 시야 반각입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Trace", meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float MaxInteractionAngle = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Trace")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_WorldDynamic;

	/** 순간 상호작용 RPC 연타 제한입니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Interaction|Network", meta = (ClampMin = "0.0"))
	float MinServerInteractionInterval = 0.1f;

private:
	/*
	 * 클라이언트 탐색 및 UI
	 */
	void PerformInteractionCheck();
	void CouldntFindInteractable();
	void FoundNewInteractable(UWorldItemWidgetComponent* WorldItemWidgetComponent, AZWorldItem* WorldItem);

	void StartLocalInteractionFeedback();
	void StopLocalInteractionFeedback();

	bool IsActorOutOfRange(const AZWorldItem* WorldItem) const;
	bool IsActorInSight(const AZWorldItem* WorldItem) const;

	/*
	 * 서버 권위 처리
	 */
	bool ValidateInteractionTarget(AZWorldItem* CandidateTarget) const;
	void BeginInteractionAuthority(AZWorldItem* CandidateTarget);
	void CompleteInteractionAuthority();
	void CancelInteractionAuthority();
	float GetInteractionDuration(AZWorldItem* Target) const;

	/*
	 * 디버그
	 */
	void DrawDebugInfo(const FVector& Start, const FVector& End, const TArray<FHitResult>& Hits, bool bAnyHit) const;

	UWorldItemWidgetComponent* GetCurrentWorldItemWidgetComponent() const { return LocalInteractionData.CurrentItemComponent.Get(); }
	AZWorldItem* GetCurrentWorldItem() const { return LocalInteractionData.CurrentWorldItem.Get(); }

private:
	/** 서버에서만 사용하는 상호작용 완료 타이머입니다. */
	FTimerHandle TimerHandle_ServerInteraction;

	/** 서버에서 순간 상호작용 RPC 연타를 제한합니다. */
	float LastServerInteractionRequestTime = -TNumericLimits<float>::Max();
};