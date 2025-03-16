// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZAttributeComponent.h"
#include "ZGameInstance.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZAttributeComponent)


/*
- **`TAutoConsoleVariable`**을 사용하여 `DamageMultiplier`라는 **전역 콘솔 변수**를 정의하고, 이 변수를 콘솔 명령어로 조정하여 **데미지 계산에 영향을 미치도록** 하는 구조입니다.
- **`GetValueOnGameThread()`**는 **멀티스레드 환경에서 안전하게 값을 가져오는 방법**이며, 게임 스레드에서 현재 값을 확인하여 게임 로직에 사용할 수 있습니다.
- 이 방식은 **디버깅, 테스트 또는 치트 모드**에서 전역 데미지 배수를 조정하는 데 유용하게 사용됩니다.
*/
static TAutoConsoleVariable<float> CVarDamageMultiplier(TEXT("game.DamageMultiplier"), 1.0f, TEXT("Global Damage Modifier for Attribute Component."), ECVF_Cheat);


// Sets default values for this component's properties
UZAttributeComponent::UZAttributeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
	this->CurrentHP = 100;
	this->CurrentBetaEnergy = 100;
	this->CurrentShield = 100;
	this->CurrentMana = 100;
	this->CurrentStamina = 100;
}


// Called when the game starts
void UZAttributeComponent::BeginPlay()
{
	Super::BeginPlay();

	// 임시땜빵 : PlayerState에서 업데이트 해줄것, SaveGameSubsystem과 연동
	FCharacterStats PlayerStats;
	PlayerStats.MaxHP = 100.f;
	PlayerStats.MaxMana = 100.f;
	PlayerStats.MaxBetaEnergy = 100;
	PlayerStats.Shield = 100;
	PlayerStats.MaxStamina = 200.f;
	InitializeCharacterStats(PlayerStats);
	
}

FCharacterStats UZAttributeComponent::GetFCharacterStats()
{
	return CharacterStats;
}

void UZAttributeComponent::InitializeCharacterStats(FCharacterStats PlayerStats)
{
	this->CharacterStats = PlayerStats;

	this->CurrentHP = PlayerStats.MaxHP;
	this->CurrentBetaEnergy = PlayerStats.MaxBetaEnergy;
	this->CurrentShield = PlayerStats.Shield;
	this->CurrentMana = PlayerStats.MaxMana;
	this->CurrentStamina = PlayerStats.MaxStamina;
}

bool UZAttributeComponent::ChangeCurrentHP(UObject* InstigatorActor, float DeltaHP)
{
	if (!GetOwner()->CanBeDamaged() && DeltaHP < 0.0f)
	{
		return false;
	}

	if (InstigatorActor == GetOwner())
	{
		return false;
	}

	if (DeltaHP < 0.0f)
	{
		const float DamageMultiplier = CVarDamageMultiplier.GetValueOnGameThread();
		DeltaHP *= DamageMultiplier;
	}

	float OldHealth = CurrentHP;
	float NewHealth = FMath::Clamp(CurrentHP + DeltaHP, 0.0f, CharacterStats.MaxHP);

	float ActualDelta = NewHealth - OldHealth;

	// Is Server?
	//if (GetOwner()->HasAuthority())
	//{
	//	CurrentHP = NewHealth;

	//	if (!FMath::IsNearlyZero(ActualDelta))
	//	{
	//		// 🎯 Delegate 호출: UI에서 HP 변화 감지
	//		OnHpChanged.Broadcast(Cast<AActor>(InstigatorActor), this, CurrentHP, ActualDelta);
	//	}

	//	if (!FMath::IsNearlyZero(ActualDelta))
	//	{
	//		MulticastHealthChanged(InstigatorActor, CurrentHP, ActualDelta);
	//	}

	//	// Died
	//	if (ActualDelta < 0.0f && FMath::IsNearlyZero(CurrentHP))
	//	{
	//		ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
	//		if (GM)
	//		{
	//			GM->OnActorKilled(GetOwner(), InstigatorActor);
	//		}
	//	}
	//}

	//ClientCode : ServerCode추가하면 삭제 
	CurrentHP = NewHealth;
	OnHpChanged.Broadcast(Cast<AActor>(InstigatorActor), this, NewHealth, ActualDelta);

	return !FMath::IsNearlyZero(ActualDelta);
}

bool UZAttributeComponent::ChangeCurrentMana(UObject* InstigatorActor, float DeltaMana)
{
	//
	float OldMana = CurrentMana;
	float NewMana = FMath::Clamp(CurrentMana + DeltaMana, 0.0f, CharacterStats.MaxMana);

	float ActualDelta = NewMana - OldMana;

	// Is Server?
	if (GetOwner()->HasAuthority())
	{
		CurrentMana = NewMana;

		if (!FMath::IsNearlyZero(ActualDelta))
		{
			// 🎯 Delegate 호출: UI에서 HP 변화 감지
			OnManaChanged.Broadcast(Cast<AActor>(InstigatorActor), this, CurrentMana, ActualDelta);
		}

		//if (!FMath::IsNearlyZero(ActualDelta))
		//{
		//	MulticastHealthChanged(InstigatorActor, CurrentHP, ActualDelta);
		//}

		//// Died
		//if (ActualDelta < 0.0f && FMath::IsNearlyZero(CurrentHP))
		//{
		//	ASGameModeBase* GM = GetWorld()->GetAuthGameMode<ASGameModeBase>();
		//	if (GM)
		//	{
		//		GM->OnActorKilled(GetOwner(), InstigatorActor);
		//	}
		//}
	}

	return !FMath::IsNearlyZero(ActualDelta);
}

bool UZAttributeComponent::ChangeCurrentBetaEnergy(UObject* InstigatorActor, float DeltaBetaEnergy)
{
	return false;
}

float UZAttributeComponent::GetCurrentHpPercent() const
{
	if (CharacterStats.MaxHP <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentHP / CharacterStats.MaxHP, 0.0f, 1.0f);
}


