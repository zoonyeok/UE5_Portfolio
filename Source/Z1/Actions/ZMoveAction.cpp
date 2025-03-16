// Fill out your copyright notice in the Description page of Project Settings.


#include "ZMoveAction.h"
#include "InputActionValue.h"
#include "Z1Character.h"
#include "GameFramework/Character.h"

UZMoveAction::UZMoveAction()
{
}

void UZMoveAction::StartAction(AActor* Instigator)
{
	Super::StartAction(Instigator);
}

void UZMoveAction::StopAction(AActor* Instigator)
{
	Super::StopAction(Instigator);
}

void UZMoveAction::Move(const FInputActionValue& Value)
{
	if (!IsRunning()) return;

	TObjectPtr<AZ1Character> Character = Cast<AZ1Character>(GetCharacter());
	if (!IsValid(Character) || !Character->GetController()) return;
	
	// 입력 값 가져오기
	FVector2D MovementVector = Value.Get<FVector2D>();

	// 현재 캐릭터의 회전 가져오기
	const FRotator Rotation = Character->Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	// 앞/옆 이동 벡터 계산
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	// 이동 입력 적용
	Character->AddMovementInput(ForwardDirection, MovementVector.Y);
	Character->AddMovementInput(RightDirection, MovementVector.X);
}
