// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ZEffect.generated.h"

class UZEffectComponent;

UENUM(BlueprintType)
enum class EZEffectDurationPolicy : uint8
{
	Instant,      // 즉시 실행 후 활성 목록에 남지 않음
	HasDuration,  // Duration 동안 유지
	Infinite,     // 명시적으로 제거되기 전까지 유지
};

UENUM(BlueprintType)
enum class EZEffectStackPolicy : uint8
{
	Independent,     // 재적용 시 항상 새 인스턴스 생성
	RefreshDuration, // 기존 효과의 종료 시각만 갱신
	IncreaseStack,   // 기존 효과의 중첩 수 증가(+ 필요 시 기간 갱신)
};

UENUM(BlueprintType)
enum class EZEffectRemovalReason : uint8
{
	Expired,    // 지속시간 만료
	Removed,    // 명시적 제거 요청(정화 등)
	Dispelled,  // 정화류 효과에 의한 제거
	Death,      // 대상 사망에 의한 정리
};

UENUM(BlueprintType)
enum class EZEffectApplicationResult : uint8
{
	Rejected,
	NewlyApplied,
	DurationRefreshed,
	StackIncreased,
	ExecutedInstant,
};

USTRUCT(BlueprintType)
struct FZEffectContext
{
	GENERATED_BODY()

	// 효과를 부여한 공격자
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TWeakObjectPtr<AActor> SourceActor;

	// 무기·투사체·불 영역 등 직접적인 출처
	UPROPERTY(BlueprintReadWrite, Category = "Context")
	TWeakObjectPtr<AActor> DamageCauser;
};

USTRUCT(BlueprintType)
struct FZEffectApplyResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	bool bSuccess = false;

	// 이후 RemoveEffect 등으로 이 효과 인스턴스를 지칭할 때 사용
	UPROPERTY(BlueprintReadOnly, Category = "Result")
	FGuid EffectHandle;

	UPROPERTY(BlueprintReadOnly, Category = "Result")
	EZEffectApplicationResult ResultType = EZEffectApplicationResult::Rejected;
};

/**
 * 효과 하나의 설정(디자이너 기본값)과 실행 상태를 담당한다.
 * 등록·중첩·갱신·제거는 UZEffectComponent가 담당하므로,
 * 외부에서는 항상 UZEffectComponent::ApplyEffect()/RemoveEffect()를 통해서만 다뤄야 한다.
 *
 * 독·화상처럼 동작이 같은 효과는 별도 C++ 클래스 없이 같은 파생 클래스의
 * Blueprint 기본값만 다르게 설정해서 구현한다. (예: UZEffect_DamageOverTime)
 */
UCLASS(Abstract, Blueprintable)
class Z1_API UZEffect : public UObject
{
	GENERATED_BODY()

public:
	FGuid GetEffectHandle() const { return EffectHandle; }
	FGameplayTag GetEffectTag() const { return EffectTag; }
	int32 GetStackCount() const { return StackCount; }
	bool IsActive() const { return bIsActive; }

	double GetRemainingTime(double ServerNow) const;

protected:
	// 최초 적용 시 동작 (Modifier 등록 등)
	virtual void OnApplied();

	// 즉시 효과 또는 주기 효과의 실행 내용
	virtual void ExecuteEffect();

	// 중첩 변경에 따른 수치 재계산
	virtual void OnStackChanged(int32 OldStackCount);

	// 자연 만료·정화·사망 등으로 제거될 때 정리.
	// 공통 정리(태그 반환 등)는 UZEffectComponent가 처리하므로,
	// 여기서는 효과 고유의 뒷정리만 담당한다.
	virtual void OnRemoved(EZEffectRemovalReason Reason);

	// ---------------- 설정 변수 (Blueprint 기본값) ----------------

	// 효과 종류 식별. Effect.Debuff.Poison 등. State 태그와 혼용하지 말 것.
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	FGameplayTag EffectTag;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	EZEffectDurationPolicy DurationPolicy = EZEffectDurationPolicy::Instant;

	UPROPERTY(EditDefaultsOnly, Category = "Effect", meta = (EditCondition = "DurationPolicy == EZEffectDurationPolicy::HasDuration"))
	float Duration = 0.f;

	// 주기 실행 간격. 0이면 주기 실행 없음
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	float Period = 0.f;

	// 지속 효과를 적용한 순간에도 한 번 실행할지
	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	bool bExecuteOnApplication = false;

	// 적용 중 대상에게 부여할 상태 태그. State.Poisoned 등. EffectTag와 구분할 것.
	UPROPERTY(EditDefaultsOnly, Category = "Effect|Tags")
	FGameplayTagContainer GrantedTags;

	// 대상에게 해당 태그가 있으면 적용 거부
	UPROPERTY(EditDefaultsOnly, Category = "Effect|Tags")
	FGameplayTagContainer BlockedByTags;

	UPROPERTY(EditDefaultsOnly, Category = "Effect|Stacking")
	int32 MaxStacks = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Effect|Stacking")
	EZEffectStackPolicy StackPolicy = EZEffectStackPolicy::Independent;

	UPROPERTY(EditDefaultsOnly, Category = "Effect|Stacking")
	bool bRefreshDurationOnReapply = true;

	UPROPERTY(EditDefaultsOnly, Category = "Effect|Stacking")
	bool bResetPeriodOnReapply = true;

	UPROPERTY(EditDefaultsOnly, Category = "Effect")
	bool bRemoveOnDeath = true;

	// ---------------- 런타임 변수 (이번에 적용된 인스턴스만의 상태) ----------------

	UPROPERTY(Transient)
	FGuid EffectHandle;

	UPROPERTY(Transient)
	TWeakObjectPtr<UZEffectComponent> OwningComponent;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> SourceActor;

	UPROPERTY(Transient)
	TWeakObjectPtr<AActor> DamageCauser;

	double StartServerTime = 0.0;
	double EndServerTime = 0.0;
	double NextExecutionServerTime = 0.0;

	UPROPERTY(Transient)
	int32 StackCount = 1;

	UPROPERTY(Transient)
	bool bIsActive = false;

	// TODO: 방어력 감소 등 능력치 변경 효과를 추가할 때,
	// AppliedModifierHandles(적용된 Modifier 핸들 목록)는
	// 파생 클래스 UZEffect_AttributeModifier 쪽에 추가한다.

private:
	friend class UZEffectComponent;

	void Initialize(UZEffectComponent* InOwningComponent, const FZEffectContext& Context, double ServerNow);
};
