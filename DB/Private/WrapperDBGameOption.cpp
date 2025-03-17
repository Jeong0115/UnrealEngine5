#include "WrapperDBGameOption.h"

#include "DBManager.h"

namespace WrapperDB
{
	constexpr const char* uspSelectUserGameOption = "CALL SelectUserGameOption(?)";

	ErrNo GetPlayerGameOption(TArray<FGameOptionTable>& outGameOptionTable, int32 id)
	{
		mysqlx::SqlResult result;

		ErrNo errNo(0);

		if (DBManager::GetInstance()->ExecuteQueryAndGetResult(uspSelectUserGameOption, result, id) == false)
		{
			errNo = errFailedExecuteQuery;
			return errNo;
		}

		if (DBManager::GetInstance()->FetchResults<FGameOptionTable, GameOptionArgs>(result, outGameOptionTable) == false)
		{
			errNo = errFailedApplyQueryResult;
			return errNo;
		}

		return ErrNo(0);
	}
}
