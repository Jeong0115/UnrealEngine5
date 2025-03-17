
#include "DBManager.h"



DBManager::~DBManager()
{
    CloseConnection();
}

bool DBManager::ConnectToDatabase(/*const FString& host, const FString& user, const FString& password, const FString& schema, int32 port*/)
{

    if (_session) return false;
    try {

		_session = std::make_unique<mysqlx::Session>("mysqlx://root:1234@127.0.0.1:33060/rpgworld");
		_session->sql("CALL CreateTempTable()").execute();



    }
    catch (const mysqlx::Error& err)
    {
        UE_LOG(LogTemp, Error, TEXT("MySQL Error : %s"), *FString(err.what()));
        return false;
    }
    catch (std::exception& ex)
    {
        UE_LOG(LogTemp, Error, TEXT("STD Exception : %s"), *FString(ex.what()));
        return false;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown error"));
        return false;
    }

    return true;

}

bool DBManager::InsertIntoDatabase(const FString& tableName, const FString& userName, const FString& password)
{
	return false;
}

void DBManager::CloseConnection()
{
	//if (_session)
	//{
	//	delete _session;
	//	_session = nullptr;
	//	UE_LOG(LogTemp, Warning, TEXT("Database session closed."));
	//}

        // 스키마 해제
       //if (_schema)
       //{
       //    delete _schema;
       //    _schema = nullptr;
       //    UE_LOG(LogTemp, Warning, TEXT("Database schema released."));r
       //}
}

bool DBManager::IsSuccessQuery(mysqlx::Row& row, unsigned long index)
{
    if (row.colCount() <= index)
    {
        UE_LOG(LogTemp, Warning, TEXT("잘못된 인덱스 접근입니다."));
        return false;
    }

    if (row[index].getType() != mysqlx::Value::Type::INT64)
    {
        UE_LOG(LogTemp, Warning, TEXT("Query 결과 상태가 int형이 아닙니다."));
        return false;
    }

    if (row[index].get<int>() != 0)
    {
        return false;
    }

    return true;
}

void DBManager::GetMessage(mysqlx::Row& row, unsigned long index, std::wstring& outMessage)
{
    if (row.colCount() <= index)
    {
        outMessage = L"잘못된 인덱스 접근입니다.";
    }

    if (row[index].getType() != mysqlx::Value::Type::STRING)
    {
        outMessage = L"Query Message가 string이 아닙니다.";
        return;
    }

    outMessage = row[index].get<std::wstring>();
}

void HandleDBException()
{
    try
    {
        throw;
    }
    catch (const mysqlx::Error& err)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs"), err.what());
    }
    catch (const std::exception& ex)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs"), ex.what());
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("DB Unknown Error"));
    }
}
