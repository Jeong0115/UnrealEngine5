#include "WrapperDB.h"
#include "DBManager.h"

namespace WrapperDB
{

	DB_API void ConnectToDatabase(/*const FString& host, const FString& user, const FString& password, const FString& schema, int32 port*/)
	{
		DBManager::GetInstance()->ConnectToDatabase(/*host, user, password, schema, port*/);
	}


}
