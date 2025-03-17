// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "Global.h"
#include "SpawnGroupInfoSubsystem.generated.h"

struct FSpawnMonsterData
{
	FSpawnMonsterData(MonsterKey key, FVector location, FRotator rotation, FVector scale)
		: _monsterKey(key), _spawnLocation(location), _spawnRotation(rotation), _spawnScale(scale) {}

	MonsterKey	_monsterKey;
	FVector		_spawnLocation;
	FRotator	_spawnRotation;
	FVector		_spawnScale;

	bool		IsValid() const;
};

typedef struct FSpawnGroupInfo : public FInfoStruct
{
	FSpawnGroupInfo();
	FSpawnGroupInfo(FSpawnGroupInfo&&) noexcept = default;

	bool						IsValid() const;

	virtual int32				GetMemorySize() const override;

	TArray<FSpawnMonsterData>	_spawnDataArray;
	float						_spawnDuration;
} SpawnGroupInfo;

using DynamicSpawnGroupInfo = DynamicInfo<SpawnGroupKey, SpawnGroupInfo>;

UCLASS()
class RPGWORLD_API USpawnGroupInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void					Initialize(FSubsystemCollectionBase& collection) override;
	virtual void					Deinitialize() override;

	virtual void					ValidateInfoData(const FString& filePath) override;

	void							LoadDynamicInfo(const SpawnGroupKey key);
	DynamicSpawnGroupInfo			GetInfo(const SpawnGroupKey key);

private:
	TSharedPtr<SpawnGroupInfo>		MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

	TMap<SpawnGroupKey, TWeakPtr<SpawnGroupInfo>> _spawnGroupInfoMap;
	
};