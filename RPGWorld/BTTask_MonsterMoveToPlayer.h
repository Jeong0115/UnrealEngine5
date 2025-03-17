// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_MonsterMoveToPlayer.generated.h"

/**
 * 
 */
UCLASS()
class RPGWORLD_API UBTTask_MonsterMoveToPlayer : public UBTTask_MoveTo
{
	GENERATED_BODY()
	
public:
	UBTTask_MonsterMoveToPlayer();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& ownerComp , uint8* nodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& ownerComp, uint8* nodeMemory, float deltaSeconds) override;
};
