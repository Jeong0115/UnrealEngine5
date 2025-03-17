// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include <type_traits>
#include "CommonStruct.generated.h"

#define STATIC_ASSERT_CONSTRUCTIBLE(T, Args) \
    static_assert(std::is_constructible_v<T, Args>, "구조체 수정 시, Args도 함께 수정해야 됩니다.");

USTRUCT()
struct FQuestTable
{
	GENERATED_BODY()

	FQuestTable()
		: _questId{}
		, _questType(0)
		, _questState(0)
		, _conditionCount(0)
	{}

	explicit FQuestTable(const int32 questId, const int32 questType, const int32 questState, const int32 conditionCount)
		: _questId(questId)
		, _questType(questType)
		, _questState(questState)
		, _conditionCount(conditionCount)
	{}

	UPROPERTY() 
	int32 _questId;

	UPROPERTY()
	int32 _questType;

	UPROPERTY()
	int32 _questState;

	UPROPERTY()
	int32 _conditionCount;
};
#define QuestTableArgs int32, int32, int32, int32
STATIC_ASSERT_CONSTRUCTIBLE(FQuestTable, QuestTableArgs)


struct FGameOptionTable
{
	int32 _currentInventorySize;
};
#define GameOptionArgs int32
STATIC_ASSERT_CONSTRUCTIBLE(FGameOptionTable, GameOptionArgs)


USTRUCT()
struct FInventoryTable
{
	GENERATED_BODY()

	FInventoryTable()
		: _slotIndex(-1)
		, _itemKey(-1)
		, _itemCount(0)
	{}

	explicit FInventoryTable(const int32 slotIndex, const int32 itemKey, const int32 itemCount)
		: _slotIndex(slotIndex)
		, _itemKey(itemKey)
		, _itemCount(itemCount)
	{}

	UPROPERTY()
	int32 _slotIndex;

	UPROPERTY()
	int32 _itemKey;

	UPROPERTY()
	int32 _itemCount;
};
#define InventoryTableArgs int32, int32, int32
STATIC_ASSERT_CONSTRUCTIBLE(FInventoryTable, InventoryTableArgs)