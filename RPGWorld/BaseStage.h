// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "Global.h"
#include "BaseStage.generated.h"

class ABaseMonster;
class AController;

UCLASS()
class RPGWORLD_API ABaseStage : public ALevelScriptActor
{
	GENERATED_BODY()
	
protected:
	virtual void					BeginPlay() override;

public:
	void							DeathMonster(ABaseMonster* monster, AController* instigator);
	void							ReturnMonsterPool(ABaseMonster* monster);

	FVector							GetVillagePosition() const { return _villagePos; }

private:
	void							CreateMonsterPool();
	void							SpawnMonster(const int32 index);

	UPROPERTY()
	TArray<ABaseMonster*>			_monsterPool;
	TArray<TArray<ABaseMonster*>>	_spawnMonsterPool;

	UPROPERTY(EditDefaultsOnly)
	int32							_stageKeyRaw = -1;

	TArray<FTimerHandle>			_spawnTimerHandleArray;
	TArray<float>					_spawnDurationArray;

	FVector							_villagePos;
	StageKey						_stageKey;

};
