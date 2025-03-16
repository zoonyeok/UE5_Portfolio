// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/ZAICharacter.h"
#include "BrainComponent.h"
#include "ZAIController.h"
#include "Components/ZAttributeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

// Sets default values
AZAICharacter::AZAICharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	SocketName = "Muzzle_01";

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	PawnSensingComp = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	AttributeComp = CreateDefaultSubobject<UZAttributeComponent>(TEXT("AttributeComp"));
	
	LockOnWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidget"));
	LockOnWidget->SetWidgetClass(LockOnWidgetClass);
	LockOnWidget->SetupAttachment(RootComponent); 	// 루트 컴포넌트(캐릭터의 머리 위 등)에 부착
	LockOnWidget->SetWidgetSpace(EWidgetSpace::Screen); // 위젯이 항상 플레이어를 향하도록 설정
	LockOnSize = FVector2d(14.f, 14.f);
	LockOnWidget->SetDrawSize(LockOnSize);
}

void AZAICharacter::BeginPlay()
{
	Super::BeginPlay();

	if (LockOnWidget)
	{
		// 메쉬의 중심 위치 계산
		FVector Center = GetMesh()->Bounds.Origin;
		LockOnWidget->SetWorldLocation(Center);
	}
	ShowLockOnWidget(false);
}

void AZAICharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	PawnSensingComp->OnSeePawn.AddDynamic(this, &AZAICharacter::OnPawnSeen);
	AttributeComp->OnHpChanged.AddDynamic(this, &AZAICharacter::OnHealthChanged);
}

void AZAICharacter::OnPawnSeen(APawn* Pawn)
{
	TObjectPtr<AZAIController> AIController = Cast<AZAIController>(GetController());
	if (IsValid(AIController))
	{
		TObjectPtr<UBlackboardComponent> BlackBoard = AIController->GetBlackboardComponent();
		if (IsValid(BlackBoard))
		{
			//BlackBoard->SetValueAsVector("MoveToLocation", Pawn->GetActorLocation());
			
			BlackBoard->SetValueAsObject("TargetActor", Pawn);
			DrawDebugString(GetWorld(), GetActorLocation(), "PLAYER SPOTTED", nullptr, FColor::White,4.0f, true);
		}
	}
}

void AZAICharacter::OnHealthChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta)
{
	if (Delta <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("AZAICharacter CurrentHealth is %f"), NewValue);

		if (NewValue <= 0.0f)
		{
			// Stop BT
			TObjectPtr<AZAIController> AIController = Cast<AZAIController>(GetController());
			if (IsValid(AIController))
			{
				AIController->GetBrainComponent()->StopLogic("Killed");
			}


			// if (IsValid(DeathAnimMontage))
			// {
			// 	PlayAnimMontage(DeathAnimMontage);
			// }
			// else
			// {
			// 	UE_LOG(LogTemp, Warning, TEXT("DeathAnimMontage is NULL"));
			// }

			UE_LOG(LogTemp, Warning, TEXT("DeathAnimMontage is NULL"));

			//Ragdoll
			EnableRagdoll();
			
			//Set lifeSpan
			SetLifeSpan(10.f);
		}
	}
}

void AZAICharacter::EnableRagdoll()
{
	// ✅ 1. SkeletalMesh에 물리 시뮬레이션 활성화
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));  // 충돌 프로필 변경
	GetMesh()->SetAllBodiesSimulatePhysics(true);  // 물리 활성화
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->WakeAllRigidBodies();  // 모든 물리 바디 활성화
	GetMesh()->bBlendPhysics = true;  // 애니메이션에서 물리로 부드럽게 전환

	// ✅ 2. 캡슐 콜리전 비활성화 (Ragdoll이 캡슐과 충돌하지 않도록)
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AZAICharacter::ShowLockOnWidget(bool bShow)
{
	if (LockOnWidget)
	{
		LockOnWidget->SetVisibility(bShow);
	}
}
