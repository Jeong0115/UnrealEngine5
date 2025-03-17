// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "Global.h"
#include "ItemInfoSubsystem.generated.h"

class UTexture2D;

typedef struct FRPGItemInfo : public FInfoStruct
{
	FRPGItemInfo() = default;
	FRPGItemInfo(FRPGItemInfo&&) noexcept = default;

	bool						IsValid() const;

	virtual int32				GetMemorySize() const override;

	TSoftObjectPtr<UTexture2D>	_itemTexture;
	FString						_name;
	FString						_desc;
	FString						_useFunc;
	int32						_maxStack;
	bool						_bUseEffect;
	bool						_bCanUseInven;
} RPGItemInfo;

using DynamicItemInfo = DynamicInfo<ItemKey, RPGItemInfo>;

UCLASS()
class RPGWORLD_API UItemInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
public:
	virtual void					Initialize(FSubsystemCollectionBase& collection) override;
	virtual void					Deinitialize() override;

	virtual void					ValidateInfoData(const FString& filePath) override;

	void							LoadDynamicInfo(const ItemKey key);
	DynamicItemInfo					GetInfo(const ItemKey key);

private:
	TSharedPtr<RPGItemInfo>			MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

	TMap<ItemKey, TWeakPtr<RPGItemInfo>> _itemInfoMap;
};
