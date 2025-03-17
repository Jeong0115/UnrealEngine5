// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

class ABasePlayer;

UCLASS()
class RPGWORLD_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
    AMonsterAIController();

protected:
    virtual void    BeginPlay() override;
    virtual void    OnPossess(APawn* inPawn) override;

public:
    virtual void    Tick(float deltaSeconds) override;

    void            StopBehaviorTree();
    void            RestartBehaviorTree();
    void            SetDetectedPlayer(ABasePlayer* player);

    bool            IsDetectedPlayer() const { return _bDetectedPlayer; }

private:
    UPROPERTY(EditDefaultsOnly, Category = "AI")
    UBehaviorTree*  _behaviorTree;

    ABasePlayer*    _detectedPlayer;
    bool		    _bDetectedPlayer;

};
