// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameInfoSubsystem.h"
#include "Global.h"
#include "StoreInfoSubsystem.generated.h"

typedef struct FItemPrice
{
	explicit FItemPrice(const ItemKey key, const int32 price)
		: _itemKey(key)
		, _price(price)
	{}

	bool	IsValid() const;

	ItemKey _itemKey;
	int32	_price;

} ItemPrice;

typedef struct FStoreInfo : public FInfoStruct
{
	FStoreInfo() = default;
	FStoreInfo(FStoreInfo&&) noexcept = default;

	bool				IsValid() const;

	virtual int32		GetMemorySize() const override;

	TArray<ItemPrice>	_itemPriceArray;
} StoreInfo;

using DynamicStoreInfo = DynamicInfo<StoreKey, StoreInfo>;

UCLASS()
class RPGWORLD_API UStoreInfoSubsystem : public UGameInfoSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void				Initialize(FSubsystemCollectionBase& collection) override;
	virtual void				Deinitialize() override;

	virtual void				ValidateInfoData(const FString& filePath) override;

	void						LoadDynamicInfo(const StoreKey key);

	DynamicStoreInfo			GetInfo(const StoreKey key);

private:
	TSharedPtr<StoreInfo>		MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject);

	TMap<StoreKey, TWeakPtr<StoreInfo>> _storeInfoMap;
	
	
};
