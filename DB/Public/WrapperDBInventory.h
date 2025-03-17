#pragma once

#include "CoreMinimal.h"

#include <string>

#include "Common\Public\Common.h"

namespace WrapperDB
{
	DB_API ErrNo SaveInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId);
	DB_API ErrNo LoadInventory(TArray<FInventoryTable>& outInventoryTableArray, const int32 userId);

	DB_API ErrNo RewardItemToInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId);
	DB_API ErrNo PurchaseItemToInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId);
	DB_API ErrNo SortInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId);
	DB_API ErrNo UseItemInventory(const int32 userId, const int32 slotIndex, const int32 itemKeyRaw, const int32 itemCount);
	// DB_API 
}	