// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseStage.h"

#include "BaseMonster.h"
#include "BasePlayerController.h"
#include "StageInfoSubsystem.h"
#include "SpawnGroupInfoSubsystem.h"
#include "MonsterInfoSubsystem.h"
#include "RPGWorldGameMode.h"
#include "TimerManager.h"

void ABaseStage::BeginPlay()
{
	Super::BeginPlay();

	_stageKey.SetKeyRawValue(_stageKeyRaw);

	const DynamicStageInfo stageInfo = GetInfoSubsystem(UStageInfoSubsystem)->GetInfo(_stageKey);
	_villagePos = stageInfo->_villagePos;

	if (HasAuthority() == false)
	{
		return;
	}

	CreateMonsterPool();

	const int32 poolSize = _spawnMonsterPool.Num();

	_spawnTimerHandleArray.SetNum(poolSize);

	for (int32 i = 0; i < poolSize; ++i)
	{
		FTimerDelegate timerDelegate;
		timerDelegate.BindLambda([i, this] { SpawnMonster(i); });

		GetWorldTimerManager().SetTimer(_spawnTimerHandleArray[i], timerDelegate, _spawnDurationArray[i], true);
	}

}

void ABaseStage::DeathMonster(ABaseMonster* monster, AController* instigator)
{
	ensureMsgfDebug(HasAuthority() == true,		TEXT("서버에서 호출해 주세요."));
	ensureMsgfDebug(monster->IsDead() == true,	TEXT("몬스터가 Dead상태가 아닙니다."));

	ABasePlayerController* playerController = Cast<ABasePlayerController>(instigator);
	if (playerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("보상 지급할 컨트롤러를 플레이어 컨트롤러로 캐스트를 실패하였습니다."));
		return;
	}

	Cast<ARPGWorldGameMode>(GetWorld()->GetAuthGameMode())->RewardPlayer(playerController, monster->GetMonsterKey());
}

void ABaseStage::ReturnMonsterPool(ABaseMonster* monster)
{
	ensureMsgfDebug(HasAuthority() == true,								TEXT("서버에서 호출해 주세요."));
	ensureMsgfDebug(monster->IsDead() == true,							TEXT("몬스터가 Dead상태가 아닙니다."));
	ensureMsgfDebug(monster->GetPoolIndex() < _spawnMonsterPool.Num(),	TEXT("비정상적인 MonsterPool Index : [%d] 입니다."), monster->GetPoolIndex());

	const int32 poolIndex = monster->GetPoolIndex();
	if (_spawnMonsterPool[poolIndex].Find(monster) == INDEX_NONE)
	{
		_spawnMonsterPool[poolIndex].Add(monster);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("풀에 존재하는데 다시 들어가려고 하네... 코드 수정 필"));
	}

}

void ABaseStage::CreateMonsterPool()
{
	if ( HasAuthority() == false )
	{
		return;
	}	

	const DynamicStageInfo stageInfo = GetInfoSubsystem(UStageInfoSubsystem)->GetInfo(_stageKey);

	const TArray<SpawnGroupKey>& spawnGroupKeyArray = stageInfo->_spawnGroupKeyArray;

	_spawnMonsterPool.SetNum(spawnGroupKeyArray.Num());
	_spawnDurationArray.Reserve(spawnGroupKeyArray.Num());
	_monsterPool.Reserve(stageInfo->_totalSpawnCount);

	int32 poolIndex = 0;
	for (const SpawnGroupKey key : spawnGroupKeyArray)
	{
		const DynamicSpawnGroupInfo spawnGroupInfo = GetInfoSubsystem(USpawnGroupInfoSubsystem)->GetInfo(key);
		_spawnDurationArray.Add(spawnGroupInfo->_spawnDuration);

		for (const auto& spawnData : spawnGroupInfo->_spawnDataArray)
		{
			const DynamicMonsterInfo monsterInfo = GetInfoSubsystem(UMonsterInfoSubsystem)->GetInfo(spawnData._monsterKey);

			TSubclassOf<ABaseMonster> monsterClass(monsterInfo->_monsterClass.LoadSynchronous());

			const FTransform spawnTransform(spawnData._spawnRotation, spawnData._spawnLocation, spawnData._spawnScale);

			ABaseMonster* monster = GetWorld()->SpawnActorDeferred<ABaseMonster>(monsterClass, spawnTransform);
			monster->SetMonsterKey(spawnData._monsterKey);
			monster->SetSpawnInfo(spawnTransform);
			monster->SetAIMoveRadius(monsterInfo->_aiMoveRadius);
			monster->SetAttackDamage(monsterInfo->_attackDamage);
			monster->SetMaxHealth(monsterInfo->_health);
			monster->SetPoolIndex(poolIndex);

			monster->FinishSpawning(spawnTransform);

			_monsterPool.Add(monster);
		}

		++poolIndex;
	}

}

void ABaseStage::SpawnMonster(const int32 index)
{
	ensureMsgfDebug(index < _spawnMonsterPool.Num(), TEXT("비정상적인 MonsterPool Index : [%d] 입니다."), index);

	TArray<ABaseMonster*> temp = _spawnMonsterPool[index];
	_spawnMonsterPool[index].Reset();

	for (ABaseMonster* monster : temp)
	{
		monster->Respawn();
	}
}
