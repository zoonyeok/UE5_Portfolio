// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ZPlayerMovementComponent.h"
#include "Z1Character.h"
#include "Animations/ZAnimInstance.h"

UZPlayerMovementComponent::UZPlayerMovementComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
}

void UZPlayerMovementComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UZPlayerMovementComponent::BeginPlay()
{
	Super::BeginPlay();
	//PlayerAnimInstance = GetAnimInstance();
	//check(PlayerAnimInstance);
}

TObjectPtr<UZAnimInstance> UZPlayerMovementComponent::GetAnimInstance()
{
	if (AZ1Character* Player = Cast<AZ1Character>(GetCharacterOwner()))
	{
		if (USkeletalMeshComponent* MeshComp = Player->GetMesh())
		{
			if (UZAnimInstance* AnimInstance = Cast<UZAnimInstance>(MeshComp->GetAnimInstance()))
			{
				return AnimInstance;
			}
		}
	}
	return nullptr;
}

//float UZPlayerMovementComponent::GetMaxSpeed() const
//{
//	return 0.0f;
//}
//
//float UZPlayerMovementComponent::GetMaxAcceleration() const
//{
//	return 0.0f;
//}