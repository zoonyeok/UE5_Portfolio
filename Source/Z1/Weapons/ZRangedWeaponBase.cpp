// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/ZRangedWeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

DEFINE_LOG_CATEGORY_STATIC(LogBaseWeapon, All, All);

AZRangedWeaponBase::AZRangedWeaponBase()
    : Super()
{
    PrimaryActorTick.bCanEverTick = false;

    // ItemCategory = EItemCategory::RangedWeapon;
    //WeaponSocketName = ""
}

void AZRangedWeaponBase::BeginPlay()
{
    Super::BeginPlay();

    check(SkeletalMeshComponent);
    checkf(DefaultAmmo.Bullets > 0, TEXT("Bullets count couldn't be less or equal zero"));
    checkf(DefaultAmmo.Clips > 0, TEXT("Clips count couldn't be less or equal zero"));
    CurrentAmmo = DefaultAmmo;

    //TODO
    //InitializeItem
}

void AZRangedWeaponBase::StartAttack()
{
    FireInProgress = true;
}

void AZRangedWeaponBase::EndAttack()
{
    FireInProgress = false;
}

bool AZRangedWeaponBase::IsFiring() const
{
    return FireInProgress;
}

void AZRangedWeaponBase::ChangeClip()
{
    if (!CurrentAmmo.Infinite)
    {
        if (CurrentAmmo.Clips == 0)
        {
            UE_LOG(LogBaseWeapon, Warning, TEXT("No more clips"));
            return;
        }
        CurrentAmmo.Clips--;
    }
    CurrentAmmo.Bullets = DefaultAmmo.Bullets;
    // UE_LOG(LogBaseWeapon, Display, TEXT("------ Change Clip ------"));
}

bool AZRangedWeaponBase::CanReload() const
{
    return CurrentAmmo.Bullets < DefaultAmmo.Bullets && CurrentAmmo.Clips > 0;
}

bool AZRangedWeaponBase::TryToAddAmmo(int32 ClipsAmount)
{
    if (CurrentAmmo.Infinite || IsAmmoFull() || ClipsAmount <= 0) return false;

    if (IsAmmoEmpty())
    {
        CurrentAmmo.Clips = FMath::Clamp(ClipsAmount, 0, DefaultAmmo.Clips + 1);
        OnClipEmpty.Broadcast(this);
    }
    else if (CurrentAmmo.Clips < DefaultAmmo.Clips)
    {
        const auto NextClipsAmount = CurrentAmmo.Clips + ClipsAmount;
        if (DefaultAmmo.Clips - NextClipsAmount >= 0)
        {
            CurrentAmmo.Clips = NextClipsAmount;
        }
        else
        {
            CurrentAmmo.Clips = DefaultAmmo.Clips;
            CurrentAmmo.Bullets = DefaultAmmo.Bullets;
        }
    }
    else
    {
        CurrentAmmo.Bullets = DefaultAmmo.Bullets;
    }

    return true;
}

bool AZRangedWeaponBase::IsAmmoEmpty() const
{
    return !CurrentAmmo.Infinite && CurrentAmmo.Clips == 0 && IsClipEmpty();
}

bool AZRangedWeaponBase::IsAmmoFull() const
{
    return CurrentAmmo.Clips == DefaultAmmo.Clips &&  //
        CurrentAmmo.Bullets == DefaultAmmo.Bullets;
}

void AZRangedWeaponBase::MakeShot()
{
}

bool AZRangedWeaponBase::GetTraceData(FVector& TraceStart, FVector& TraceEnd) const
{
    FVector ViewLocation;
    FRotator ViewRotation;
    if (!GetPlayerViewPoint(ViewLocation, ViewRotation)) return false;

    TraceStart = ViewLocation;
    const FVector ShootDirection = ViewRotation.Vector();
    TraceEnd = TraceStart + ShootDirection * TraceMaxDistance;
    return true;
}

bool AZRangedWeaponBase::GetPlayerViewPoint(FVector& ViewLocation, FRotator& ViewRotation) const
{
    const auto STUCharacter = Cast<ACharacter>(GetOwner());
    if (!STUCharacter) return false;

    if (STUCharacter->IsPlayerControlled())
    {
        const auto Controller = STUCharacter->GetController<APlayerController>();
        if (!Controller) return false;

        Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
    }
    else
    {
        ViewLocation = GetMuzzleWorldLocation();
        ViewRotation = SkeletalMeshComponent->GetSocketRotation(MuzzleSocketName);
    }

    return true;
}

FVector AZRangedWeaponBase::GetMuzzleWorldLocation() const
{
    return SkeletalMeshComponent->GetSocketLocation(MuzzleSocketName);
}

void AZRangedWeaponBase::MakeHit(FHitResult& HitResult, const FVector& TraceStart, const FVector& TraceEnd)
{
    if (!GetWorld()) return;

    FCollisionQueryParams CollisionParams;
    CollisionParams.bReturnPhysicalMaterial = true;
    CollisionParams.AddIgnoredActor(GetOwner());

    GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECollisionChannel::ECC_Visibility, CollisionParams);
}

void AZRangedWeaponBase::DecreaseAmmo()
{
    if (CurrentAmmo.Bullets == 0)
    {
        UE_LOG(LogBaseWeapon, Warning, TEXT("Clip is empty"));
        return;
    }
    CurrentAmmo.Bullets--;

    if (IsClipEmpty() && !IsAmmoEmpty())
    {
        EndAttack();
        OnClipEmpty.Broadcast(this);
    }
}

bool AZRangedWeaponBase::IsClipEmpty() const
{
    return CurrentAmmo.Bullets == 0;
}

void AZRangedWeaponBase::LogAmmo()
{
    FString AmmoInfo = "Ammo: " + FString::FromInt(CurrentAmmo.Bullets) + " / ";
    AmmoInfo += CurrentAmmo.Infinite ? "Infinite" : FString::FromInt(CurrentAmmo.Clips);
    UE_LOG(LogBaseWeapon, Display, TEXT("%s"), *AmmoInfo);
}

UNiagaraComponent* AZRangedWeaponBase::SpawnMuzzleFX()
{
    return UNiagaraFunctionLibrary::SpawnSystemAttached(MuzzleFX,  //
        SkeletalMeshComponent,                                                //
        MuzzleSocketName,                                          //
        FVector::ZeroVector,                                       //
        FRotator::ZeroRotator,                                     //
        EAttachLocation::SnapToTarget, true);
}

EInventoryActionResult AZRangedWeaponBase::PickUp(APawn* Player)
{
    return Super::PickUp(Player);
}

UZInventoryItem* AZRangedWeaponBase::ConvertToInventoryItem()
{
    return Super::ConvertToInventoryItem();
}
