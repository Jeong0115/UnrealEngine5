#pragma once

#include "CoreMinimal.h"

#include <string>

#include "Common\Public\Common.h"

namespace WrapperDB
{
	DB_API ErrNo SelectUserQuest(TArray<FQuestTable>& outQuestTable, int32 id);
	DB_API ErrNo UpdateQuestCondition(const TArray<FQuestTable>& questTableArray, const int32 userId);
	DB_API ErrNo AcceptQuest(const int32 userId, const int32 questId, const int32 questType, const int32 clearCount);
	DB_API ErrNo ClearQuest(const int32 userId, const int32 questId);
	// DB_API 
}	