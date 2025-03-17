#pragma once

#include "CoreMinimal.h"

#include <string>
#include "Common\Public\Common.h"


namespace WrapperDB
{
	DB_API ErrNo CreateAccount(const std::string& id, const std::string& password, TSharedPtr<FString> outResult);
	DB_API ErrNo LoginAccount(const std::string& id, const std::string& password, TSharedPtr<FString> outResult);
}