// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_MonsterMoveToPlayer.h"

#include "BehaviorTree/BlackboardComponent.h"

#include "MonsterAIController.h"
#include "BaseMonster.h"

UBTTask_MonsterMoveToPlayer::UBTTask_MonsterMoveToPlayer()
{
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_MonsterMoveToPlayer::ExecuteTask(UBehaviorTreeComponent& ownerComp , uint8* nodeMemory)
{
	UBlackboardComponent* blackboard = ownerComp.GetBlackboardComponent();
	if ( blackboard == nullptr )
	{
		return EBTNodeResult::Failed;
	}

	AcceptableRadius = blackboard->GetValueAsFloat("AcceptableRadius");

	return Super::ExecuteTask(ownerComp , nodeMemory);
}

void UBTTask_MonsterMoveToPlayer::TickTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds)
{
	Super::TickTask(ownerComp, nodeMemory, deltaSeconds);

	UBlackboardComponent* blackboard = ownerComp.GetBlackboardComponent();
	FVector spawnLocation = blackboard->GetValueAsVector("SpawnLocation");

	AAIController* aiController = ownerComp.GetAIOwner();
	if (aiController == nullptr)
	{
		return;
	}

	AMonsterAIController* controller = Cast<AMonsterAIController>(aiController);
	if (controller == nullptr)
	{
		return;
	}

	APawn* pawn = controller->GetPawn();
	if (pawn == nullptr)
	{
		return;
	}

	FVector actorLocation = pawn->GetActorLocation();

	float distance = FVector::Dist(actorLocation, spawnLocation);
	if (distance > 800.f)
	{
		controller->SetDetectedPlayer(nullptr);
		FinishLatentTask(ownerComp, EBTNodeResult::Failed);
	}
}
