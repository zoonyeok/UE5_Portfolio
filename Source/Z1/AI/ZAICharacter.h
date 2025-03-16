// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ZAICharacter.generated.h"

class UPawnSensingComponent;
class UZAttributeComponent;
class UWidgetComponent;

UCLASS()
class Z1_API AZAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AZAICharacter();

	FORCEINLINE FVector GetSocketLocation() const { return GetMesh()->GetSocketLocation(SocketName); }
	FORCEINLINE FName GetWeaponSocketName() const { return SocketName; }
	FORCEINLINE TSubclassOf<AActor> GetProjectileClass() const { return ProjectileClass; }

	void ShowLockOnWidget(bool bShow);

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category=Animation)
	TObjectPtr<UAnimMontage> DeathAnimMontage; 
	
	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	FName SocketName;

	UPROPERTY(EditDefaultsOnly, Category = Weapon)
	TSubclassOf<AActor> ProjectileClass;

	UPROPERTY(VisibleAnywhere, Category = AI)
	UPawnSensingComponent* PawnSensingComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UZAttributeComponent> AttributeComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UWidgetComponent> LockOnWidget;

	UPROPERTY(EditDefaultsOnly, Category = UI)
	TSubclassOf<UUserWidget> LockOnWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = UI, meta = (AllowPrivateAccess = "true"))
	FVector2D LockOnSize;
	
	UFUNCTION(BlueprintCallable, Category = AI)
	void OnPawnSeen(APawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = Attribute)
	void OnHealthChanged(AActor* InstigatorActor, UZAttributeComponent* OwningComp, float NewValue, float Delta);

	void EnableRagdoll();
};

