// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/ZRangedAttackProjectile.h"
#include "Components/SphereComponent.h"
#include "Components/ZAttributeComponent.h"

AZRangedAttackProjectile::AZRangedAttackProjectile()
	: Super()
{
	SphereComp->SetSphereRadius(20.0f);
}

void AZRangedAttackProjectile::BeginPlay()
{
	Super::BeginPlay();
	SphereComp->OnComponentBeginOverlap.AddDynamic(this, &AZRangedAttackProjectile::OnActorOverlap);
}

void AZRangedAttackProjectile::OnActorOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != GetInstigator())
	{
		TObjectPtr<UZAttributeComponent> AttributeComp = Cast<UZAttributeComponent>(OtherActor->GetComponentByClass(UZAttributeComponent::StaticClass()));
		if (IsValid(AttributeComp))
		{
			// minus in front of DamageAmount to apply the change as damage, not healing
			AttributeComp->ChangeCurrentHP(this, -DamageAmount);

			// Only explode when we hit something valid
			Explode();
		}
	}
}
