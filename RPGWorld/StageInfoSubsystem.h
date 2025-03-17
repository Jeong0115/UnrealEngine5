// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "Global.h"
#include "StageInfoSubsystem.generated.h"

typedef struct FStageInfo : public FInfoStruct
{
	FStageInfo()
		: _spawnGroupKeyArray{}
		, _name{}
		, _villagePos(FVector::ZeroVector)
		, _totalSpawnCount(0)
	{}

	FStageInfo(FStageInfo&&) noexcept = default;

	bool					IsValid();

	virtual int32			GetMemorySize() const override;

	TArray<SpawnGroupKey>	_spawnGroupKeyArray;
	FString					_name;
	FVector					_villagePos;
	int32					_totalSpawnCount;
} StageInfo;

using DynamicStageInfo = DynamicInfo<StageKey, StageInfo>;

UCLASS()
class RPGWORLD_API UStageInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void			Initialize(FSubsystemCollectionBase& collection) override;
	virtual void			Deinitialize() override;
	
	virtual void			ValidateInfoData(const FString& filePath) override;

	void					LoadDynamicInfo(const StageKey key);
	DynamicStageInfo		GetInfo(const StageKey key);

private:
	TSharedPtr<StageInfo>	MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

	TMap<StageKey, TWeakPtr<StageInfo>> _stageInfoMap;
};
