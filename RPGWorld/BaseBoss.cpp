#include "BaseBoss.h"

#include "RPGWorldGameMode.h"

#include "BaseStage.h"
#include "MonsterAIController.h"
#include "TimerManager.h"

void ABaseBoss::Die(AController* damageInstigator)
{
	ABaseCharacter::Die(damageInstigator);

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

    Cast<ABaseStage>(GetLevel()->GetLevelScriptActor())->DeathMonster(this, damageInstigator);
    Cast<ARPGWorldGameMode>(GetWorld()->GetAuthGameMode())->DeathBossMonster(this, damageInstigator);

    const int32 count = _deathAnimationArray.Num();
    const int32 randIndex = FMath::RandRange(0, count - 1);

    PlayDeathAnimation(randIndex);

    GetWorldTimerManager().SetTimer(_deathTimerHandle, this, &ABaseBoss::DestoryBoss, 1.5f, false);
}

void ABaseBoss::DestoryBoss()
{
    if (HasAuthority() == false)
    {
        return;
    }

    if (IsDead() == false)
    {
        return;
    }

    Destroy();
}
