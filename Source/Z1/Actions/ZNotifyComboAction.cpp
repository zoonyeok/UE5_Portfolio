// Fill out your copyright notice in the Description page of Project Settings.


#include "Actions/ZNotifyComboAction.h"
#include "Z1Character.h"
#include "Animations/ZAnimInstance.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/ZComboActionData.h"

UZNotifyComboAction::UZNotifyComboAction()
{
	
}

void UZNotifyComboAction::StartAction(AActor* Instigator)
{
	// 부모 로직: bIsRunning=true, GrantsTags 적용 등
	Super::StartAction(Instigator);

	// 콤보 입력 로직으로 연결
	// (첫 공격 시도)
	ProcessComboCommand();
}

void UZNotifyComboAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ComboTimerHandle);
	}

	if (UZAnimInstance* AnimInstance = GetOwningAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			// 델리게이트를 먼저 해제 (이전 콜백이 실행되지 않도록)
			FOnMontageEnded EndDelegate;
			AnimInstance->Montage_SetEndDelegate(EndDelegate, nullptr);

			AnimInstance->Montage_Stop(0.2f, AttackMontage);
		}
	}

	ComboTimerHandle.Invalidate();

	CurrentCombo = 1;
	bHasNextComboCommand = false;
	bInputConsumed = false;
}

void UZNotifyComboAction::ProcessComboCommand()
{
	//if (bInputConsumed) // 1. 입력 중복 차단
	//{
	//	UE_LOG(LogTemp, Warning, TEXT("Input already consumed!"));
	//	return;
	//}

	// 1) Montage / Data 유효성 먼저 체크
	if (!AttackMontage || !ComboActionData)
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid AttackMontage or ComboActionData"));
		return;
	}

	// 2) 콤보 미시작(=0)이면 첫 콤보 시작
	if (CurrentCombo == 1 && bInputConsumed == false)
	{
		bInputConsumed = true; // 2. 첫 입력 마킹
		ComboActionBegin();
		return;
	}

	// 3) 이미 콤보가 진행 중이면 → 타이머가 살아있는지 확인
	if (ComboTimerHandle.IsValid())
	{
		// 아직 콤보 윈도우가 열려 있다면, 한 번만 HasNextComboCommand = true 설정
		if (!bHasNextComboCommand)
		{
			bHasNextComboCommand = true;
			UE_LOG(LogTemp, Log, TEXT("Next combo input accepted! (CurrentCombo=%d)"), CurrentCombo);
		}
		else
		{
			// 이미 다음 콤보 예약됨 -> 스팸 입력 무시
			UE_LOG(LogTemp, Log, TEXT("Next combo input already set, ignoring spam. (CurrentCombo=%d)"), CurrentCombo);
		}
	}
	else
	{
		StopAction(GetCharacter());
	}
}

void UZNotifyComboAction::ComboActionBegin()
{
	// 첫 콤보 시작
	bHasNextComboCommand = false;

	if (UZAnimInstance* AnimInstance = GetOwningAnimInstance())
	{
		if (AnimInstance->Montage_IsPlaying(AttackMontage))
		{
			return; // 이미 실행 중이면 중복 실행 방지
		}

		// FOnMontageEnded EndDelegate;
		// AnimInstance->Montage_SetEndDelegate(EndDelegate, nullptr);

		AnimInstance->Montage_Play(AttackMontage);
		
		UE_LOG(LogTemp, Log, TEXT("ComboActionBegin!"));
		
		// EndDelegate.BindUObject(this, &UZNotifyComboAction::ComboActionEnd);
		// AnimInstance->Montage_SetEndDelegate(EndDelegate, AttackMontage);

		SetComboCheckTimer();
	}
}

void UZNotifyComboAction::ComboActionEnd(UAnimMontage* InAttackMontage, bool bProperlyEnded)
{
	//ensure(CurrentCombo != 1);

	if (CurrentCombo < ComboActionData->MaxComboCount && bInputConsumed && bHasNextComboCommand && ComboTimerHandle.IsValid())
	{
		return;
	}
	// 액션 종료
	StopAction(GetCharacter());
}

void UZNotifyComboAction::SetComboCheckTimer()
{
	// 콤보 체크 타이머 초기화
	ComboTimerHandle.Invalidate();

	// 콤보 인덱스는 (CurrentCombo - 1)기준으로 EffectiveFrameCount 참조
	int32 ComboIndex = CurrentCombo - 1;
	if (!ComboActionData->EffectiveFrameCount.IsValidIndex(ComboIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ComboIndex: %d"), ComboIndex);
		return;
	}

	 // 프레임 -> 시간 변환
	 float MinComboTime = 0.1f;
	 //float ComboEffectiveTime = FMath::Max(ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate, MinComboTime);
	
	 float CurrentFrameRate = 1.0f / GetWorld()->GetDeltaSeconds();
	 float ComboEffectiveTime = ComboActionData->EffectiveFrameCount[ComboIndex] / ComboActionData->FrameRate;
	 //AttackSpeedRate 등을 고려하려면 여기서 나누기/곱하기

	// 현재 섹션의 남은 시간 계산
	/*float SectionLength = AttackMontage->GetSectionLength(CurrentCombo - 1);
	float ElapsedTime = AnimInstance->Montage_GetPosition(AttackMontage);
	float RemainingTime = SectionLength - ElapsedTime;*/

	 if (ComboEffectiveTime > 0.0f)
	 {
	 	if (UWorld* World = GetWorld())
	 	{
	 		World->GetTimerManager().SetTimer(
	 			ComboTimerHandle,
	 			this,
	 			&UZNotifyComboAction::ComboCheck,
	 			ComboEffectiveTime,
	 			false
	 		);
	 	}
	 }
}

void UZNotifyComboAction::ComboCheck()
{
	if (bHasNextComboCommand && CurrentCombo < ComboActionData->MaxComboCount)
	{
		if (UZAnimInstance* AnimInstance = GetOwningAnimInstance())
		{
			CurrentCombo++;
			FName NextSection = *FString::Printf(TEXT("%s%d"), *ComboActionData->MontageSectionNamePrefix, CurrentCombo);

			// 현재 섹션이 끝난 후 다음 섹션으로 연결
			AnimInstance->Montage_JumpToSection(NextSection, AttackMontage);
			
			UE_LOG(LogTemp, Log, TEXT("Next combo input accepted!, NextSection"));
			bInputConsumed = false; // 3. 다음 입력 허용
			bHasNextComboCommand = false;
			SetComboCheckTimer();
		}
	}
	 else
	 {
	 	StopAction(GetCharacter());
	 }
}


