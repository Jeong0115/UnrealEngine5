// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseMonster.h"

#include "Components\SphereComponent.h"
#include "Components\CapsuleComponent.h"
#include "Components\BoxComponent.h"

#include "Engine\World.h"
#include "Engine\OverlapResult.h"
#include "Net\UnrealNetwork.h"
#include "Kismet\GameplayStatics.h"
#include "TimerManager.h"

#include "BasePlayer.h"
#include "BaseStage.h"
#include "MonsterAIController.h"

#include "MinimapSubsystem.h"

ABaseMonster::ABaseMonster()
    : _bCollisionEnabled(true)
    , _spawnTransform(FTransform::Identity)
    , _detectPlayerRadius(500.f)
    , _aiMoveRadius(100.f)
{
    PrimaryActorTick.bCanEverTick = false;  

    bUseControllerRotationRoll = false;
    bUseControllerRotationPitch = false;

    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    _attackCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Collision"));
    _attackCollision->SetupAttachment(RootComponent);
}

void ABaseMonster::BeginPlay()
{
    Super::BeginPlay();

    _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

    if (_attackAnimationArray.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("몬스터 공격 애니메이션을 1개 이상 등록해주세요"));
    }

    if (_deathAnimationArray.Num() <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("몬스터 죽음 애니메이션을 1개 이상 등록해주세요"));
    }

    if (HasAuthority() == true)
    {
        _bCollisionEnabled = true;
        if (_attackCollision)
        {
            _attackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABaseMonster::OnOverlapAttackCollision);
        }
    }
    else
    {
        GetGameInstance()->GetSubsystem<UMinimapSubsystem>()->RegisterCharacterOnMinimap(ECharacterType::Monster, this);
    }
       
}

void ABaseMonster::Destroyed()
{
    Super::Destroyed();

    if (HasAuthority() == false)
    {
        GetGameInstance()->GetSubsystem<UMinimapSubsystem>()->RemoveCharacterOnMinimap(ECharacterType::Monster, this);
    }
}

void ABaseMonster::Tick(float deltaTime)
{
    Super::Tick(deltaTime);

    if (HasAuthority() == false)
    {
        return;
    }

    AMonsterAIController* monsterAIController = Cast<AMonsterAIController>(GetController());
    if (monsterAIController == nullptr)
    {
        return;
    }

    if (monsterAIController->IsDetectedPlayer() == false)
    {
        DetectPlayer();
    }
}

void ABaseMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABaseMonster, _bCollisionEnabled);
}

void ABaseMonster::Die(AController* damageInstigator)
{
    Super::Die(damageInstigator);

    if (HasAuthority() == false)
    {
        return;
    }

    if (IsDead() == false)
    {
        return;
    }

    AMonsterAIController* monsterAIController = Cast<AMonsterAIController>(GetController());
    if (monsterAIController)
    {
        monsterAIController->StopBehaviorTree();
    }

    _bCollisionEnabled = false;
     
    SetActorEnableCollision(_bCollisionEnabled);
    SetAttackCollisionEnabled(false);

    Cast<ABaseStage>(GetLevel()->GetLevelScriptActor())->DeathMonster(this, damageInstigator);

    const int32 count       = _deathAnimationArray.Num();
    const int32 randIndex   = FMath::RandRange(0, count - 1);

    PlayDeathAnimation(randIndex);

    GetWorldTimerManager().SetTimer(_deathTimerHandle, this, &ABaseMonster::ReturnMonsterPool, 1.5f, false);
}

void ABaseMonster::Respawn(float spawnHpRatio)
{
    if (HasAuthority() == false)
    {
        return;
    }

    AMonsterAIController* monsterAIController = Cast<AMonsterAIController>(GetController());
    if (monsterAIController)
    {
        monsterAIController->RestartBehaviorTree();
    }

    Super::Respawn(spawnHpRatio);
    
    BPRespawn();

    _bCollisionEnabled  = true;
    ResetMaterialParmeter();
    SetActorEnableCollision(_bCollisionEnabled);
    SetActorTransformSafe(_spawnTransform);

    SetActorHiddenInGame(false);
    SetActorTickEnabled(true);
}

void ABaseMonster::SetSpawnInfo(const FTransform& transform)
{
    _spawnTransform = transform;
}

void ABaseMonster::SetAttackCollisionEnabled(bool bEnabled)
{
    if (HasAuthority() == false)
    {
        return;
    }

    if (bEnabled == true)
    {
        _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
    }
    else
    {
        _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
        _attackedPlayerSet.Empty();
    }

}

void ABaseMonster::ReturnMonsterPool()
{
    ensureMsgfDebug(HasAuthority() == true, TEXT("서버에서 호출해 주세요"));
    
    ResetMaterialParmeter();

    SetActorHiddenInGame(true);
    UpdateComponentVisibility();
    SetActorTickEnabled(false);

    Cast<ABaseStage>(GetLevel()->GetLevelScriptActor())->ReturnMonsterPool(this);
}

void ABaseMonster::OnRep_SetCollisionEnabled()
{
    SetActorEnableCollision(_bCollisionEnabled);
}

void ABaseMonster::OnOverlapAttackCollision(UPrimitiveComponent*    overlappedComponent, 
                                            AActor*                 otherActor, 
                                            UPrimitiveComponent*    otherComp, 
                                            int32                   otherBodyIndex, 
                                            bool                    bFromSweep, 
                                            const FHitResult&       sweepResult)
{
    if (_attackedPlayerSet.Contains(otherActor) == true)
    {
        return;
    }

    UGameplayStatics::ApplyDamage(otherActor, _attackDamage, GetInstigatorController(), this, UDamageType::StaticClass());

    _attackedPlayerSet.Add(otherActor);
}

void ABaseMonster::DetectPlayer()
{
    ensureMsgfDebug(HasAuthority() == true, TEXT("서버에서 호출 해주세요"));

    TArray<FOverlapResult> overlapResultArray;
    const FVector location = GetActorLocation();

    const FCollisionShape sphere = FCollisionShape::MakeSphere(_detectPlayerRadius);

    if (GetWorld()->OverlapMultiByChannel(overlapResultArray, location,FQuat::Identity, ECC_GameTraceChannel7, sphere))
    {
        for (const auto& result : overlapResultArray)
        {
            ABasePlayer* player = Cast<ABasePlayer>(result.GetActor());
            if (player == nullptr || player->IsDead() == true)
            {
                continue;
            }

            float distance = FVector::Dist(_spawnTransform.GetLocation(), player->GetActorLocation());
            if (distance > 800.f)
            {
                continue;
            }

            AMonsterAIController* monsterAIController = Cast<AMonsterAIController>(GetController());
            monsterAIController->SetDetectedPlayer(player);

            break;
        }

    }
}
