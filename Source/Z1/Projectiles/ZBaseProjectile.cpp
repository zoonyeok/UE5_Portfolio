// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/ZBaseProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/FastReferenceCollector.h"

// Sets default values
AZBaseProjectile::AZBaseProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SphereComp = CreateDefaultSubobject<USphereComponent>("SphereComp");
	SphereComp->SetCollisionProfileName("Projectile");
	RootComponent = SphereComp;

	EffectComp = CreateDefaultSubobject<UParticleSystemComponent>("EffectComp");
	EffectComp->SetupAttachment(RootComponent);

	MoveComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMoveComp"));
	MoveComp->InitialSpeed = 2000.0f;  // 발사 속도 (1500 이상 권장)
	MoveComp->MaxSpeed = 4000.0f;      // 최대 속도
	MoveComp->bRotationFollowsVelocity = true; // 발사 방향 유지
	MoveComp->bInitialVelocityInLocalSpace = false; // 월드 기준 속도 적용
	MoveComp->ProjectileGravityScale = 0.0f; // 중력 영향 없음
	MoveComp->bShouldBounce = false; // 튕김 없음
	MoveComp->bAutoActivate = true; // 자동 활성화
}

void AZBaseProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComp->OnComponentHit.AddDynamic(this, &AZBaseProjectile::OnActorHit);
}

void AZBaseProjectile::OnActorHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	Explode();
}

// _Implementation from it being marked as BlueprintNativeEvent
void AZBaseProjectile::Explode_Implementation()
{
	// Check to make sure we aren't already being 'destroyed'
	// Adding ensure to see if we encounter this situation at all
	if (IsValid(this))
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactVFX, GetActorLocation(), GetActorRotation());

		EffectComp->DeactivateSystem();

		MoveComp->StopMovementImmediately();
		SetActorEnableCollision(false);

		Destroy();
	}
}

void AZBaseProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();
}
