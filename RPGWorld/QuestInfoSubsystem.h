// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "Global.h"
#include "QuestInfoSubsystem.generated.h"

enum class EQuestState : uint8
{
	Accept,
	Complete,
	Clear,
	None
};

enum class EConditionType : uint8
{
	KillMonster,
	GetItem,
	UseItem,
	Count
};
DeclConvertEnumAndString(EConditionType);

enum class ERewardType : uint8
{
	AcceptQuest,
	GiveItem,
	Count
};
DeclConvertEnumAndString(ERewardType);

enum class EQuestType : uint8
{
	Normal,
	Repeat,
	Count
};

typedef struct FClearCondition
{
	FClearCondition()
		: _type(EConditionType::Count)
		, _conditionKeyRaw(-1)
		, _clearCount(-1)
	{}

	explicit FClearCondition(EConditionType type, int32 conditionKeyRaw, int32 clearCount)
		: _type(type)
		, _conditionKeyRaw(conditionKeyRaw)
		, _clearCount(clearCount)
	{}

	bool			IsValid() const;

	EConditionType	_type;
	int32			_conditionKeyRaw;
	int32			_clearCount;
} ClearCondition;

typedef struct FQuestReward
{
	explicit FQuestReward(ERewardType type, int32 rewardKeyRaw, int32 rewardCount)
		: _type(type)
		, _rewardKeyRaw(rewardKeyRaw)
		, _rewardCount(rewardCount) 
	{}

	bool		IsValid() const;

	ERewardType	_type;
	int32		_rewardKeyRaw;
	int32		_rewardCount;
} QuestReward;

typedef struct FQuestInfo : public FInfoStruct
{
	FQuestInfo()						= default;
	FQuestInfo(FQuestInfo&&) noexcept	= default;
	
	bool				IsValid() const;

	virtual int32		GetMemorySize() const override;

	TArray<QuestReward>	_questRewardArray;
	ClearCondition		_clearCondition;

	FString				_name;
	FString				_desc;

	EQuestType			_qusetType;
	int32				_prevKey;
} QuestInfo;
	
using DynamicQuestInfo = DynamicInfo<QuestKey, QuestInfo>;

UCLASS()
class RPGWORLD_API UQuestInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void				Initialize(FSubsystemCollectionBase& collection) override;
	virtual void				Deinitialize() override;

	virtual void				ValidateInfoData(const FString& filePath) override;

	void						LoadDynamicInfo(const QuestKey key);

	DynamicQuestInfo			GetInfo(const QuestKey key);

private:
	TSharedPtr<QuestInfo>		MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

	TMap<QuestKey, TWeakPtr<QuestInfo>> _questInfoMap;
};
