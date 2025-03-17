// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "DropGroupInfoSubsystem.generated.h"

typedef struct FDropItem
{
	explicit FDropItem(int32 key, int32 count, float probability)
		: _itemKey(key), _itemCount(count), _probability(probability)
	{}

	bool IsValid() const;

	ItemKey _itemKey;
	int32	_itemCount;
	float	_probability;

} DropItem;

typedef struct FDropGroupInfo : public FInfoStruct
{
	FDropGroupInfo() = default;
	FDropGroupInfo(FDropGroupInfo&&) noexcept = default;

	bool					IsValid() const;

	virtual int32			GetMemorySize() const override;

	const TArray<DropItem>& GetDropItemArray() const { return _dropItemArray; }

private:
	TArray<DropItem>		_dropItemArray;

	friend class UDropGroupInfoSubsystem;
} DropGroupInfo;


using DynamicDropGroupInfo = DynamicInfo<DropGroupKey, DropGroupInfo>;

UCLASS()
class RPGWORLD_API UDropGroupInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
public:
	virtual void					Initialize(FSubsystemCollectionBase& collection) override;
	virtual void					Deinitialize() override;

	virtual void					ValidateInfoData(const FString& filePath) override;

	void							LoadDynamicInfo(const DropGroupKey key);
	DynamicDropGroupInfo			GetInfo(const DropGroupKey key);

private:
	TSharedPtr<DropGroupInfo>		MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

	TMap<DropGroupKey, TWeakPtr<DropGroupInfo>> _dropGroupInfoMap;
};
