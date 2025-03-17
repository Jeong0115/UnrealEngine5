#pragma once

#include "CoreMinimal.h"

#include <string>

#include "Common\Public\Common.h"

namespace WrapperDB
{
	DB_API ErrNo GetPlayerGameOption(TArray<FGameOptionTable>& outGameOptionTable, int32 id);
	// DB_API 
}	