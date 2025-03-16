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
	MoveComp->InitialSpeed = 2000.0f;  // �߻� �ӵ� (1500 �̻� ����)
	MoveComp->MaxSpeed = 4000.0f;      // �ִ� �ӵ�
	MoveComp->bRotationFollowsVelocity = true; // �߻� ���� ����
	MoveComp->bInitialVelocityInLocalSpace = false; // ���� ���� �ӵ� ����
	MoveComp->ProjectileGravityScale = 0.0f; // �߷� ���� ����
	MoveComp->bShouldBounce = false; // ƨ�� ����
	MoveComp->bAutoActivate = true; // �ڵ� Ȱ��ȭ
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
