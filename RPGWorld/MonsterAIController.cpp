// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAIController.h"

#include "BehaviorTree\BehaviorTree.h"
#include "BehaviorTree\BlackboardComponent.h"

#include "BaseMonster.h"
#include "BasePlayer.h"

AMonsterAIController::AMonsterAIController()
    : _bDetectedPlayer(false)
{
	// _blackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void AMonsterAIController::BeginPlay()
{
    Super::BeginPlay();
}

void AMonsterAIController::OnPossess(APawn* inPawn)
{
    Super::OnPossess(inPawn);
       
    if (HasAuthority() == true)
    {
        if (_behaviorTree == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("MonsterAIController에 할당된 행동트리가 없습니다."));
            return;
        }

        ABaseMonster* monster = Cast<ABaseMonster>(GetPawn());
        if (monster == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("Monster Class를 상속받지 않은 캐릭터에 MonsterController가 할당되어 있습니다."));
            return;
        }

        RunBehaviorTree(_behaviorTree);

        Blackboard->SetValueAsFloat("AcceptableRadius", monster->GetAIMoveRadius());
        Blackboard->SetValueAsVector("SpawnLocation", monster->GetSpawnLocation());
    }
}

void AMonsterAIController::Tick(float deltaSeconds)
{
    Super::Tick(deltaSeconds);

    if (_detectedPlayer != nullptr)
    {
        if (_detectedPlayer->IsDead() == true)
        {
            SetDetectedPlayer(nullptr);

            return;
        }
    }
}

void AMonsterAIController::StopBehaviorTree()
{
    SetDetectedPlayer(nullptr);

    if (BrainComponent && BrainComponent->IsRunning())
    {
        BrainComponent->StopLogic(TEXT("Stop Behavior Tree"));
    }
}

void AMonsterAIController::RestartBehaviorTree()
{
    if (HasAuthority() == true)
    {
        RunBehaviorTree(_behaviorTree);
    }
}

void AMonsterAIController::SetDetectedPlayer(ABasePlayer* player)
{
    _detectedPlayer = player;

    if (_detectedPlayer != nullptr)
    {
        if (_detectedPlayer->IsDead() == true)
        {
            UE_LOG(LogTemp, Warning, TEXT("Dead상태인 플레이어가 전달되었습니다."));
            _detectedPlayer = nullptr;
            _bDetectedPlayer = false;
        }
        else
        {
            _bDetectedPlayer = true;
        }
    }
    else
    {
        _bDetectedPlayer = false;
    }

    Blackboard->SetValueAsObject("DetectedPlayer", _detectedPlayer);
}