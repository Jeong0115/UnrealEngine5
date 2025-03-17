#include "WrapperDBLogin.h"

#include "DBManager.h"

namespace WrapperDB
{
	constexpr const char* uspCreateAccount	= "CALL CreateAccount(?, ?)";
	constexpr const char* uspLoginAccount	= "CALL LoginAccount(?, ?)";

	ErrNo CreateAccount(const std::string& id, const std::string& password, TSharedPtr<FString> outResult)
	{
		std::wstring result;
		if (DBManager::GetInstance()->ExecuteQueryAndFetchResult(result, uspCreateAccount, id, password) == false)
		{
			FString str(result.c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
			return errFailedExecuteQuery;
		}

		*outResult = FString(result.c_str());

		return ErrNo(0);
	}

	ErrNo LoginAccount(const std::string& id, const std::string& password, TSharedPtr<FString> outResult)
	{
		std::wstring result;
		if (DBManager::GetInstance()->ExecuteQueryAndFetchResult(result, uspLoginAccount, id, password) == false)
		{
			FString str(result.c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
			return errFailedExecuteQuery;
		}

		*outResult = FString(result.c_str());

		return ErrNo(0);
	}
}