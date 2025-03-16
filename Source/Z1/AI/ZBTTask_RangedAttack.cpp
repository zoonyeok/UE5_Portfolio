// Fill out your copyright notice in the Description page of Project Settings.

#include "AI/ZBTTask_RangedAttack.h"
#include "AI/ZAIController.h"
#include "AI/ZAICharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"


EBTNodeResult::Type UZBTTask_RangedAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    TObjectPtr<AAIController> AIController = OwnerComp.GetAIOwner();
    if (!IsValid(AIController))
    {
        return EBTNodeResult::Failed;
    }

    TObjectPtr<AZAICharacter> MyPawn = Cast<AZAICharacter>(AIController->GetPawn());
    if (!IsValid(MyPawn))
    {
        return EBTNodeResult::Failed;
    }

    FName MuzzleSocket = MyPawn->GetWeaponSocketName();
    FVector SocketLocation = MyPawn->GetMesh()->GetSocketLocation(MuzzleSocket);
    FRotator MuzzleRotation = MyPawn->GetMesh()->GetSocketRotation(MuzzleSocket);

    TObjectPtr<UBlackboardComponent> BlackboardComponent = OwnerComp.GetBlackboardComponent();
    if (!IsValid(BlackboardComponent))
    {
        return EBTNodeResult::Failed;
    }

    TObjectPtr<AActor> TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject("TargetActor"));
    if (!IsValid(TargetActor))
    {
        return EBTNodeResult::Failed;
    }

    // ✅ 발사 방향 계산
    FVector Direction = (TargetActor->GetActorLocation() - SocketLocation).GetSafeNormal();
    MuzzleRotation = Direction.Rotation();

    // ✅ Projectile 스폰 파라미터 설정
    FActorSpawnParameters SpawnParameters;
    SpawnParameters.Owner = MyPawn;
    SpawnParameters.Instigator = MyPawn;
    SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // ✅ Projectile 생성
    TObjectPtr<AActor> Projectile = GetWorld()->SpawnActor<AActor>(MyPawn->GetProjectileClass(), SocketLocation, MuzzleRotation, SpawnParameters);
    if (IsValid(Projectile))
    {
        // ✅ AI 자신과 충돌하지 않도록 설정 (더 안전한 방식으로 수정)
        if (UPrimitiveComponent* ProjectileCollision = Cast<UPrimitiveComponent>(Projectile->GetRootComponent()))
        {
            ProjectileCollision->IgnoreActorWhenMoving(MyPawn, true);
        }

        // ✅ 발사 방향 설정
        FVector LaunchDirection = MuzzleRotation.Vector();

        // ✅ ProjectileMovementComponent 가져오기 및 속도 적용
        TObjectPtr<UProjectileMovementComponent> ProjectileMovement = Projectile->FindComponentByClass<UProjectileMovementComponent>();
        if (IsValid(ProjectileMovement))
        {
            ProjectileMovement->Velocity = LaunchDirection * ProjectileMovement->InitialSpeed;
        }
    }

    return IsValid(Projectile) ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}
