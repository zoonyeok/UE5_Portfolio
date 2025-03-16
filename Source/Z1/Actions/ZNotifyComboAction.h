#pragma once

#include "CoreMinimal.h"
#include "Actions/ZAction.h"
#include "ZNotifyComboAction.generated.h"

class UZComboActionData;
UCLASS()
class Z1_API UZNotifyComboAction  : public UZAction
{
	GENERATED_BODY()
	
public:
	UZNotifyComboAction();
	
	virtual void StartAction(AActor* Instigator) override;
	virtual void StopAction(AActor* Instigator) override;
	
	// 콤보 입력이 들어왔을 때 호출 (키 이벤트 등)
	UFUNCTION(BlueprintCallable)
	void ProcessComboCommand();

protected:
	// 콤보를 실제 시작(첫 공격)하는 함수
	UFUNCTION()
	void ComboActionBegin();

	// 몽타주가 끝날 때(마지막 공격 종료 시점) 호출
	UFUNCTION()
	void ComboActionEnd(UAnimMontage* InAttackMontage, bool bProperlyEnded);

	// "입력 가능 시간" 타이머를 세팅
	void SetComboCheckTimer();

	// 타이머 만료 후, 다음 콤보 섹션으로 이동할지 말지 결정
	UFUNCTION()
	void ComboCheck();

protected:
	// 콤보 데이터(프레임 수, 섹션 prefix 등)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TObjectPtr<UZComboActionData> ComboActionData;

	// 재생할 몽타주
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combo")
	TObjectPtr<UAnimMontage> AttackMontage;

	// 현재 콤보 인덱스(0이면 비활성)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combo")
	int32 CurrentCombo = 1;

	// 다음 콤보를 진행할 명령(입력)이 들어왔는지 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combo")
	bool bHasNextComboCommand = false;

	bool bInputConsumed = false; // 새로 추가된 입력 소비 플래그

	// 콤보 입력 유효 시간 체크 타이머
	FTimerHandle ComboTimerHandle;
	
};

