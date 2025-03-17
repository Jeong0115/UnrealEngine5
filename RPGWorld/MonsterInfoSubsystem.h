// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "Global.h"
#include "MonsterInfoSubsystem.generated.h"

class ABaseMonster;
class FJsonObject;

typedef struct FMonsterInfo : public FInfoStruct
{
	FMonsterInfo() = default;
	FMonsterInfo(FMonsterInfo&&) noexcept = default;

	bool						IsValid();

	virtual int32				GetMemorySize() const override;

	TSoftClassPtr<ABaseMonster>	_monsterClass;
	DropGroupKey				_dropGroupKey;
	FString						_name;
	float						_aiMoveRadius;
	int32						_health;
	int32						_attackDamage;
	int32						_level;
} MonsterInfo;

using DynamicMonsterInfo = DynamicInfo<MonsterKey, MonsterInfo>;

UCLASS()
class RPGWORLD_API UMonsterInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
public:
	virtual void				Initialize(FSubsystemCollectionBase& collection) override;
	virtual void				Deinitialize() override;

	virtual void				ValidateInfoData(const FString& filePath) override;

	void						LoadDynamicInfo(const MonsterKey key);
	DynamicMonsterInfo			GetInfo(const MonsterKey key);

private:
	TSharedPtr<MonsterInfo>		MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

private:
	TMap<MonsterKey, TWeakPtr<MonsterInfo>> _monsterInfoMap;
};
