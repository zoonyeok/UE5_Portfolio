// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZAttributeComponent.h"
#include "Net/UnrealNetwork.h"

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
	SetIsReplicatedByDefault(true);

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
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FCharacterStats PlayerStats{};
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

void UZAttributeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UZAttributeComponent, CurrentHP);
	DOREPLIFETIME(UZAttributeComponent, CurrentMana);
	DOREPLIFETIME(UZAttributeComponent, CurrentStamina);
	DOREPLIFETIME(UZAttributeComponent, CurrentBetaEnergy);
	DOREPLIFETIME(UZAttributeComponent, CurrentShield);
	DOREPLIFETIME(UZAttributeComponent, AttackPower);
	DOREPLIFETIME(UZAttributeComponent, ShieldAttackPower);
	DOREPLIFETIME(UZAttributeComponent, CharacterStats);
}

void UZAttributeComponent::InitializeCharacterStats(FCharacterStats PlayerStats)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	CharacterStats = PlayerStats;
	CurrentHP = FMath::Max(0.f, PlayerStats.MaxHP);
	CurrentMana = FMath::Max(0.f, PlayerStats.MaxMana);
	CurrentStamina = FMath::Max(0.f, PlayerStats.MaxStamina);
	CurrentBetaEnergy = FMath::Max(0.f, PlayerStats.MaxBetaEnergy);
	CurrentShield = FMath::Max(0.f, PlayerStats.Shield);
	AttackPower = PlayerStats.Attack;
	ShieldAttackPower = PlayerStats.ShieldAttack;
	
	OnCharacterStatsChanged.Broadcast();
}

bool UZAttributeComponent::ChangeCurrentHP(UObject* InstigatorActor, float DeltaHP)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !FMath::IsFinite(DeltaHP))
	{
		return false;
	}
	if ((!GetOwner()->CanBeDamaged() && DeltaHP < 0.f) || InstigatorActor == GetOwner())
	{
		return false;
	}
	if (DeltaHP < 0.f)
	{
		DeltaHP *= CVarDamageMultiplier.GetValueOnGameThread();
	}
	if (!FMath::IsFinite(DeltaHP))
	{
		return false;
	}

	const float OldHP = CurrentHP;
	CurrentHP = FMath::Clamp(CurrentHP + DeltaHP, 0.f, FMath::Max(0.f, CharacterStats.MaxHP));
	const float ActualDelta = CurrentHP - OldHP;
	if (FMath::IsNearlyZero(ActualDelta))
	{
		return false;
	}
	OnHpChanged.Broadcast(Cast<AActor>(InstigatorActor), this, CurrentHP, ActualDelta);
	return true;
}

bool UZAttributeComponent::ChangeCurrentMana(UObject* InstigatorActor, float DeltaMana)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !FMath::IsFinite(DeltaMana))
	{
		return false;
	}
	const float OldMana = CurrentMana;
	CurrentMana = FMath::Clamp(CurrentMana + DeltaMana, 0.f, FMath::Max(0.f, CharacterStats.MaxMana));
	const float ActualDelta = CurrentMana - OldMana;
	if (FMath::IsNearlyZero(ActualDelta))
	{
		return false;
	}
	OnManaChanged.Broadcast(Cast<AActor>(InstigatorActor), this, CurrentMana, ActualDelta);
	return true;
}

bool UZAttributeComponent::ChangeCurrentBetaEnergy(UObject* InstigatorActor, float DeltaBetaEnergy)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !FMath::IsFinite(DeltaBetaEnergy))
	{
		return false;
	}
	const float OldEnergy = CurrentBetaEnergy;
	CurrentBetaEnergy = FMath::Clamp(CurrentBetaEnergy + DeltaBetaEnergy, 0.f,
		FMath::Max(0.f, CharacterStats.MaxBetaEnergy));
	return !FMath::IsNearlyZero(CurrentBetaEnergy - OldEnergy);
}

void UZAttributeComponent::OnRep_CurrentHP(float OldHP)
{
	// State replication carries no hit instigator and may combine several changes.
	OnHpChanged.Broadcast(nullptr, this, CurrentHP, CurrentHP - OldHP);
}

void UZAttributeComponent::OnRep_CurrentMana(float OldMana)
{
	OnManaChanged.Broadcast(nullptr, this, CurrentMana, CurrentMana - OldMana);
}

void UZAttributeComponent::OnRep_CharacterStats()
{
	OnCharacterStatsChanged.Broadcast();
}

float UZAttributeComponent::GetCurrentHpPercent() const
{
	if (CharacterStats.MaxHP <= 0)
	{
		return 0.0f;
	}

	return FMath::Clamp(CurrentHP / CharacterStats.MaxHP, 0.0f, 1.0f);
}


