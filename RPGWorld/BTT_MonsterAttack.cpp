#include "BTT_MonsterAttack.h"

#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BlackboardComponent.h"

#include "BaseMonster.h"

EBTNodeResult::Type UBTT_MonsterAttack::ExecuteTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory)
{
    AAIController* controller = ownerComp.GetAIOwner();
    if (controller == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    ABaseMonster* monster = Cast<ABaseMonster>(controller->GetPawn());
    if (monster == nullptr)
    {
        return EBTNodeResult::Failed;
    }

    const int32 count = monster->GetAttackAnimationCount();
    const int32 randIndex = FMath::RandRange(0, count - 1);

    monster->PlayAttackAnimation(randIndex);

    return EBTNodeResult::Succeeded;
}
