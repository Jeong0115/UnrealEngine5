// Fill out your copyright notice in the Description page of Project Settings.


#include "BasePlayer.h"

#include "Engine\OverlapResult.h"
#include "Camera\CameraComponent.h"
#include "Components\BoxComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework\SpringArmComponent.h"
#include "Net\UnrealNetwork.h"
#include "Kismet\GameplayStatics.h"

#include "BasePlayerController.h"

#include "MinimapSubsystem.h"

ABasePlayer::ABasePlayer()
    : _turnRate(10.f)
    , _attackDamage(20)
    , _autoStartLocation()
    , _targetEnemy(nullptr)
    , _attackRange(200.f)
    , _attackIndex(0)
    , _cameraType(ECameraType::MainCamera)
    , _bAutoBattle(false)
    , _bAutoPotion(false)
    , _bSaveAttack(false)
    , _bMoveInput(false)
    , _isAttacking(false)
    , _isAttackByAutoBattle(false)
{
    PrimaryActorTick.bCanEverTick = true;

    _springArm          = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    _mainCamera         = CreateDefaultSubobject<UCameraComponent>(TEXT("MainCamera"));
    _attackCollision    = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Collision"));

    _springArm->SetupAttachment(RootComponent);
    _springArm->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f));
    _springArm->TargetArmLength             = 1200.f;
    _springArm->bUsePawnControlRotation     = true;
    _springArm->bInheritYaw                 = false;
    _springArm->bInheritRoll                = false;
    _springArm->bInheritPitch               = false;
    _springArm->bDoCollisionTest            = false;

    _mainCamera->SetupAttachment(_springArm);
    _mainCamera->SetActive(true);
    _mainCamera->bUsePawnControlRotation = false;

    _attackCollision->SetupAttachment(RootComponent);
    _attackCollision->SetRelativeLocation(FVector(90.f, 0.f, 0.f));
    _attackCollision->SetBoxExtent(FVector(80.f, 120.f, 90.f));
    _attackCollision->SetCollisionProfileName("PlayerAttack");
    _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void ABasePlayer::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() == true)
    {
        _attackCollision->OnComponentBeginOverlap.AddDynamic(this, &ABasePlayer::OnOverlapAttackCollision);
        _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

        return;
    }

    if (IsLocallyControlled() == true)
    {
        APlayerController* playerController = Cast<APlayerController>(Controller);
        if (playerController != nullptr)
        {
            UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer());
            if (subsystem != nullptr)
            {
                subsystem->AddMappingContext(_playerInputMappingContext, 0);
            }
        }
    }
    else
    {
	    GetGameInstance()->GetSubsystem<UMinimapSubsystem>()->RegisterCharacterOnMinimap(ECharacterType::OtherPlayer, this);
    }
    
}

void ABasePlayer::Destroyed()
{
    Super::Destroyed();

    if (HasAuthority() == true)
    {
        return;
    }

    if (IsLocallyControlled() == false)
    {
        GetGameInstance()->GetSubsystem<UMinimapSubsystem>()->RemoveCharacterOnMinimap(ECharacterType::OtherPlayer, this);
    }
}

void ABasePlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (IsDead() == true)
    {
        return;
    }

    if (HasAuthority() == true)
    {
        if (_bAutoPotion == true && (GetCurrentHealth() / (float)GetMaxHealth() < 0.5f))
        {
            Cast<ABasePlayerController>(Controller)->UseAutoPotion();
        }
    }
    else if (IsLocallyControlled() == true)
    {
        if (_targetEnemy && _targetEnemy->IsDead() == true)
        {
            _targetEnemy = nullptr;
        }

        if (_bAutoBattle == true)
        {
            AutoBattle();
        }
    }
}

void ABasePlayer::SetupPlayerInputComponent(UInputComponent* playerInputComponent)
{
    Super::SetupPlayerInputComponent(playerInputComponent);
    if (UEnhancedInputComponent* inputComponent = Cast<UEnhancedInputComponent>(playerInputComponent))
    {
        if (_moveAction)
        {
            inputComponent->BindAction(_moveAction, ETriggerEvent::Triggered,   this, &ABasePlayer::MoveAction);
            inputComponent->BindAction(_moveAction, ETriggerEvent::Started,     this, &ABasePlayer::StartMoveAction);
            inputComponent->BindAction(_moveAction, ETriggerEvent::Completed,   this, &ABasePlayer::CompleteMoveAction);
        }

        if (_lookAction)
        {
            inputComponent->BindAction(_lookAction,   ETriggerEvent::Triggered, this, &ABasePlayer::LookAction);
        }

        if (_attackAction)
        {
            inputComponent->BindAction(_attackAction, ETriggerEvent::Triggered, this, &ABasePlayer::AttackAction);
        }
    }
}

void ABasePlayer::PawnClientRestart()
{
    Super::PawnClientRestart();
    if (APlayerController* controller = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(controller->GetLocalPlayer()))
        {
            subsystem->ClearAllMappings();
            subsystem->AddMappingContext(_playerInputMappingContext, 0);
        }
    }
}

void ABasePlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ABasePlayer, _isAttacking);
    DOREPLIFETIME(ABasePlayer, _bSaveAttack);
    DOREPLIFETIME(ABasePlayer, _bAutoPotion);
}

void ABasePlayer::Die(AController* damageInstigator)
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

    _targetEnemy = nullptr;
    _attackIndex = 0;
    _bSaveAttack = false;
    _isAttacking = false;

    _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);

    SetActorEnableCollision(false);

    PlayDeathAnimation();
}

void ABasePlayer::Respawn(float spawnHpRatio)
{
    Super::Respawn(spawnHpRatio);

    if (HasAuthority() == false)
    {
        return;
    }

    if (IsDead() == true)
    {
        return;
    }
    
    SetActorEnableCollision(true);
    SetActorHiddenInGame(false);

    ABasePlayerController* playerController = Cast<ABasePlayerController>(Controller);
    playerController->HideRespawnWidget();
}

void ABasePlayer::ChangeCameraType(const ECameraType type)
{
    if (HasAuthority() == true)
    {
        return;
    }

    if (_cameraType == type)
    {
        return;
    }

    const FRotator rotator = _springArm->GetRelativeRotation();

    switch (type)
    {
    case ECameraType::MainCamera:
    {
        _springArm->SetRelativeRotation(FRotator(-55.f, rotator.Yaw, rotator.Roll));
        _springArm->TargetArmLength = 1200.f;
        _springArm->SocketOffset.Z  = 0.f;
        break;
    }

    case ECameraType::VillageCamera:
    {
        _springArm->SetRelativeRotation(FRotator(0.f, rotator.Yaw, rotator.Roll));
        _springArm->TargetArmLength = 300.f;
        _springArm->SocketOffset.Z  = 100.f;
        break;
    }

    default: break;
    }

    _cameraType = type;
}

void ABasePlayer::OnOverlapAttackCollision( UPrimitiveComponent*    overlappedComponent, 
                                            AActor*                 otherActor, 
                                            UPrimitiveComponent*    otherComp, 
                                            int32                   otherBodyIndex, 
                                            bool                    bFromSweep, 
                                            const FHitResult&       sweepResult)
{
    UGameplayStatics::ApplyDamage(otherActor, _attackDamage, GetInstigatorController(), this, UDamageType::StaticClass());
}

void ABasePlayer::AttackActionRPC_Implementation()
{
    if (_isAttacking == true && _bSaveAttack == true)
    {
        //UE_LOG(LogTemp, Warning, TEXT("검증 실패. [AttackActionRPC()] _isAttacking : [%d], _bSaveAttack : [%d]"), _isAttacking, _bSaveAttack);
        return;
    }

    if (_isAttacking == true)
    {
        _bSaveAttack = true;
        return;
    }
    
    _isAttacking = true;
    PlayAttackAnimationRPC(_attackIndex++);
}

void ABasePlayer::ComboAttackActionRPC_Implementation()
{
    if (_isAttacking == false || _bSaveAttack == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("검증 실패. [ComboAttackActionRPC()] _isAttacking : [%d], _bSaveAttack : [%d]"), _isAttacking, _bSaveAttack);
        return;
    }

    _bSaveAttack = false;
    PlayAttackAnimationRPC(_attackIndex++);
}

void ABasePlayer::ResetComboRPC_Implementation()
{
    if (_isAttacking == false)
    {
        UE_LOG(LogTemp, Warning, TEXT("검증 실패. [ResetComboRPC()] _isAttacking : [%d]"), _isAttacking);
        return;
    }

    _attackIndex = 0;
    _bSaveAttack = false;
    _isAttacking = false;
}

void ABasePlayer::HidePlayerAndShowDeathWidget_Implementation()
{
    SetActorHiddenInGame(true);

    ABasePlayerController* playerController = Cast<ABasePlayerController>(Controller);
    playerController->ShowRespawnWidget();
}

void ABasePlayer::PlayAttackAnimation_Implementation(const int32 attackIndex)
{
    UE_LOG(LogTemp, Warning, TEXT("플레이어 공격 애니메이션 재생 함수를 블루프린트에서 구현해야 합니다."));
}

void ABasePlayer::PlayAttackAnimationRPC_Implementation(const int32 attackIndex)
{
    PlayAttackAnimation(attackIndex);
}

void ABasePlayer::SetAttackCollisionEnabledRPC_Implementation(bool bEnabled)
{
    if (bEnabled == true)
    {
        _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::QueryOnly);
    }
    else
    {
        _attackCollision->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
    }
}

void ABasePlayer::TurnAutoBattle()
{
    _bAutoBattle = !_bAutoBattle;
    if (_bAutoBattle == true)
    {
        _autoStartLocation = GetActorLocation();
    }
}

void ABasePlayer::TurnOffAutoBattle_Implementation()
{
    _bAutoBattle = false;
}

void ABasePlayer::TurnAutoPotion_Implementation()
{
    _bAutoPotion = !_bAutoPotion;
}


void ABasePlayer::MoveAction(const FInputActionInstance& instance)
{
    if (Controller == nullptr || IsDead() == true)
    {
        return;
    }

    FVector2D inputVector = instance.GetValue().Get<FVector2D>();

    const FRotator springArmRotation = FRotator(0.f, _springArm->GetTargetRotation().Yaw, 0.f);

    const float relativeAngle = FMath::RadiansToDegrees(FMath::Atan2(inputVector.X, inputVector.Y));

    const FRotator targetRotation(0.0f, FRotator::NormalizeAxis(springArmRotation.Yaw + relativeAngle), 0.0f);
    const FRotator currentRotation = Controller->GetControlRotation();

    const FRotator interpRotation = FMath::RInterpTo(currentRotation, targetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), _turnRate);
    Controller->SetControlRotation(interpRotation);

    AddMovementInput(FRotationMatrix(springArmRotation).GetUnitAxis(EAxis::X), inputVector.Y);
    AddMovementInput(FRotationMatrix(springArmRotation).GetUnitAxis(EAxis::Y), inputVector.X);
    
    if (_bAutoBattle == true)
    {
        _autoStartLocation = GetActorLocation();
    }
}

void ABasePlayer::LookAction(const FInputActionInstance& instance)
{
    if (Controller == nullptr || IsDead() == true)
    {
        return;
    }

    FVector2D lookVector = instance.GetValue().Get<FVector2D>();

    _springArm->AddWorldRotation(FRotator(0.f, lookVector.X, 0.f));
}

void ABasePlayer::AttackAction()
{
    if (IsDead() == true)
    {
        return;
    }

    if (_isAttacking == true && _bSaveAttack == true)
    {
        return;
    }

    AttackActionRPC();
}

void ABasePlayer::FindTargetEnemey()
{
    TArray<FOverlapResult> overlapResultArray;

    const FVector playerLocation = GetActorLocation();
    const FCollisionShape sphere = FCollisionShape::MakeSphere(600);
 
    if (GetWorld()->OverlapMultiByChannel(overlapResultArray, playerLocation, FQuat::Identity, ECC_GameTraceChannel5, sphere))
    {
        overlapResultArray.Sort([&](const FOverlapResult& left, const FOverlapResult& right)
            {
                const AActor* leftActor = left.GetActor();
                const AActor* rightActor = right.GetActor();

                if (leftActor == nullptr || rightActor == nullptr)
                {
                    return false;
                }

                const float leftDist = FVector::Dist(playerLocation, leftActor->GetActorLocation());
                const float rightDist = FVector::Dist(playerLocation, rightActor->GetActorLocation());

                return leftDist < rightDist;
            });

        for (const auto& overlapResult : overlapResultArray)
        {
  
            ABaseCharacter* overlapActor = Cast<ABaseCharacter>(overlapResult.GetActor());
            if (overlapActor == nullptr || overlapActor->IsDead() == true)
            {
                continue;
            }

            _targetEnemy = overlapActor;
            break;
        }

    }
}

void ABasePlayer::AutoBattle()
{
    if (_bMoveInput == true)
    {
        return;
    }

    if (_targetEnemy != nullptr)
    {
        if (FVector::Distance(GetActorLocation(), _targetEnemy->GetActorLocation()) <= _attackRange)
        {
            TurnToTargetEnemy();
            AttackAction();
            return;
        }
    }

    FindTargetEnemey();

    if (_targetEnemy == nullptr)
    {
        MoveToStartLocation();
        return;
    }

    TurnToTargetEnemy();
    MoveToTargetEnemy();
}

void ABasePlayer::TurnToTargetEnemy()
{
    if (Controller == nullptr)
    {
        return;
    }

    const FVector targetLocation = _targetEnemy->GetActorLocation();
    const FVector direction = (targetLocation - GetActorLocation()).GetSafeNormal();
    const FRotator targetRotation = FRotationMatrix::MakeFromX(direction).Rotator();
    const FRotator interpRotation = FMath::RInterpTo(Controller->GetControlRotation(), targetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), _turnRate);

    Controller->SetControlRotation(interpRotation);
}

void ABasePlayer::MoveToTargetEnemy()
{
    if (Controller == nullptr)
    {
        return;
    }

    const FVector targetLocation = _targetEnemy->GetActorLocation();
    const FVector direction = (targetLocation - GetActorLocation()).GetSafeNormal();

    AddMovementInput(direction);
}

void ABasePlayer::MoveToStartLocation()
{
    if (Controller == nullptr)
    {
        return;
    }

    const FVector actorLocation = GetActorLocation();
    if (FVector::Distance(_autoStartLocation, actorLocation) <= 50.f)
    {
        return;
    }

    const FVector direction = (_autoStartLocation - actorLocation).GetSafeNormal();

    AddMovementInput(direction);

    const FRotator targetRotation = FRotationMatrix::MakeFromX(direction).Rotator();
    const FRotator interpRotation = FMath::RInterpTo(Controller->GetControlRotation(), targetRotation, UGameplayStatics::GetWorldDeltaSeconds(this), _turnRate);

    Controller->SetControlRotation(interpRotation);
}
