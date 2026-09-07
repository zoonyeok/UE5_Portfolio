// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "Effects/ZEffect.h"
#include "ZEffectComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectAdded, UZEffect*, Effect);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEffectRemoved, UZEffect*, Effect, EZEffectRemovalReason, Reason);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEffectChanged, UZEffect*, Effect);

/*
 * 대상에게 적용된 효과의 추가·중첩·만료·해제를 담당한다.
 * 효과 하나의 설정·실행 내용은 UZEffect가, 등록·중첩·갱신·제거는 이 컴포넌트가 담당한다.
 * 스턴 : 신규 행동 차단, 실행 중 행동 취소, 이동 제한
 * 화상 : 주기적 피해, 효과 수명, 중첩 및 갱신
 * 둔화 : 이동 속도 Modifier와 중첩 효과 해제
 * 방어력 감소 : 피해 계산과 능력치 효과 연결
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class Z1_API UZEffectComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZEffectComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 조건 검사 -> 중첩 대상 검색 -> 신규 적용 또는 갱신
	UFUNCTION(BlueprintCallable, Category = "Effects")
	FZEffectApplyResult ApplyEffect(TSubclassOf<UZEffect> EffectClass, const FZEffectContext& Context);

	// 핸들로 특정 효과 제거. 같은 핸들로 두 번 호출해도 안전함.
	UFUNCTION(BlueprintCallable, Category = "Effects")
	bool RemoveEffect(FGuid EffectHandle, EZEffectRemovalReason Reason);

	// 정화처럼 조건에 해당하는 효과들을 제거
	UFUNCTION(BlueprintCallable, Category = "Effects")
	int32 RemoveEffectsByTag(FGameplayTagContainer EffectTags, EZEffectRemovalReason Reason);

	// 사망 시 제거 정책(bRemoveOnDeath)에 해당하는 효과 정리
	UFUNCTION(BlueprintCallable, Category = "Effects")
	int32 RemoveEffectsOnDeath();

	// 대상 해제·종료 시 전체 정리
	UFUNCTION(BlueprintCallable, Category = "Effects")
	void ClearAllEffects();

	UZEffect* FindEffect(FGuid EffectHandle) const;

	UFUNCTION(BlueprintCallable, Category = "Effects")
	bool HasEffect(FGameplayTag EffectTag) const;

	// 효과가 부여한 상태 태그(GrantedTags) 보유 여부
	UFUNCTION(BlueprintCallable, Category = "Effects")
	bool HasEffectTag(FGameplayTag Tag) const;

	const FGameplayTagContainer& GetActiveEffectTags() const { return ActiveEffectTags; }

	// 주기 실행·만료 판정
	void UpdateEffects(double ServerNow);

	// 컴포넌트 소유자와 실제 대상 Pawn이 다를 때 사용
	AActor* GetAvatarActor() const { return AvatarActor.IsValid() ? AvatarActor.Get() : GetOwner(); }
	void SetAvatarActor(AActor* NewAvatarActor) { AvatarActor = NewAvatarActor; }

	UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
	FOnEffectAdded OnEffectAdded;

	UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
	FOnEffectRemoved OnEffectRemoved;

	UPROPERTY(BlueprintAssignable, Category = "Effects|Events")
	FOnEffectChanged OnEffectChanged;

protected:
	virtual void BeginPlay() override;

	// 서버 권한·대상 상태·효과 데이터·면역 검사
	bool CanApplyEffect(TSubclassOf<UZEffect> EffectClass, const FZEffectContext& Context) const;

	// 종류와 출처 정책에 맞는 기존 효과 검색.
	// 첫 구현은 "같은 효과 종류 + 같은 SourceActor"일 때만 중첩 대상으로 본다.
	UZEffect* FindStackableEffect(TSubclassOf<UZEffect> EffectClass, const FZEffectContext& Context) const;

	// 순회 중 미뤄둔 추가·제거 처리
	void ProcessPendingChanges();

	void GrantEffectTags(UZEffect* Effect);
	void ReturnEffectTags(UZEffect* Effect);

private:
	// 활성 효과의 기준 목록·GC 참조 유지
	UPROPERTY(Transient)
	TArray<TObjectPtr<UZEffect>> ActiveEffects;

	// 순회 중 새 효과 추가를 안전하게 처리하기 위한 대기열
	UPROPERTY(Transient)
	TArray<TObjectPtr<UZEffect>> PendingApplications;

	// 순회 도중 요청된 제거를 나중에 반영하기 위한 대기열
	TArray<FGuid> PendingRemovals;

	// 현재 효과 목록 순회 중인지
	bool bIsUpdatingEffects = false;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> AvatarActor;

	// 효과들이 부여한 상태 태그의 참조 카운트.
	// ActionComponent의 ActiveGameplayTags와는 별개 경로이므로,
	// 두 시스템이 같은 태그를 함께 쓰는 경우(예: State.Stunned)는
	// 최종적으로 하나의 공통 상태 태그 관리자로 합치는 것을 고려할 것.
	TMap<FGameplayTag, int32> EffectTagCounts;

	UPROPERTY(Transient)
	FGameplayTagContainer ActiveEffectTags;
};
