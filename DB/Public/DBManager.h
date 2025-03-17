// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include <tuple>

#pragma warning(push)
#pragma warning(disable: 4265)
#include <xdevapi.h>


//#define STATIC_CONCPP
#pragma push_macro("check")
#undef check 

#include <jdbc/mysql_driver.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/driver.h>
#include <jdbc/cppconn/exception.h>

#pragma pop_macro("check")

#pragma warning(pop)

#include "Common\Public\Common.h"

#include "RPGWorld\Singleton.h"


class DBManager : public Singleton<DBManager>
{

public:
	~DBManager();

	bool ConnectToDatabase(/*const FString& host, const FString& user, const FString& password, const FString& schema, int32 port*/);
	bool InsertIntoDatabase(const FString& tableName, const FString& userName, const FString& password);

	void CloseConnection();

	template <typename... Args>
	bool ExecuteQueryAndFetchResult(std::wstring& outString, const char* query, const Args&... args);

    template <typename... Args>
    bool ExecuteQueryAndGetResult(const char* query, mysqlx::SqlResult& outResult, const Args&... args);

    template <typename T, class... Args>
    bool FetchResults(mysqlx::SqlResult& sqlResult, TArray<T>& outResult);

    bool IsSuccessQuery(mysqlx::Row& row, unsigned long index);
    void GetMessage(mysqlx::Row& row, unsigned long index, std::wstring& outMessage);

    std::unique_ptr<mysqlx::Session>& GetSession() { return _session; }

private:
	std::unique_ptr<mysqlx::Session> _session = nullptr;
};

void HandleDBException();

template <typename... Args>
bool DBManager::ExecuteQueryAndFetchResult(std::wstring& outString, const char* query, const Args&... args)
{
    try 
    {
        if (_session == nullptr)
        {
            //
        }
        
        auto stmt = _session->sql(query);
        (stmt.bind(args), ...);

        auto result = stmt.execute();
        auto row = result.fetchOne();
        
        if (row.isNull() == true || row.colCount() == 0)
        {
            outString = L"Query가 결과를 반환하지 않았습니다.";
            return false;
        }

        bool isSuccess = IsSuccessQuery(row, 0);
        GetMessage(row, 1, outString);

        return isSuccess;
    }
    catch (const mysqlx::Error& err)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs"), err.what());
        return false;
    }
    catch (std::exception& ex)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs"), ex.what());
        return false;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown Error : ExecuteQueryAndGetResult() "));
        return false;
    }

    return false;
}

template <typename... Args>
bool DBManager::ExecuteQueryAndGetResult(const char* query, mysqlx::SqlResult& outResult, const Args&... args)
{
    try 
    {
        if (_session == nullptr)
        {
            //
        }

        auto stmt = _session->sql(query);
        (stmt.bind(args), ...);

        outResult = stmt.execute();
        auto row = outResult.fetchOne();

        if (row.isNull() == true || row.colCount() == 0)
        {
            UE_LOG(LogTemp, Warning, TEXT("Query가 결과를 반환하지 않았습니다."));
            return false;
        }

        if (IsSuccessQuery(row, 0) == false)
        {
            std::wstring errMessage = L"";
            GetMessage(row, 1, errMessage);

            UE_LOG(LogTemp, Warning, TEXT("%s"), errMessage.c_str());
            return false;
        }

        outResult.nextResult();
        return true;
    }
    catch (const mysqlx::Error& err)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs"), err.what());
        return false;
    }
    catch (std::exception& ex)
    {
        UE_LOG(LogTemp, Error, TEXT("%hs"), ex.what());
        return false;
    }
    catch (...)
    {
        UE_LOG(LogTemp, Error, TEXT("Unknown Error : ExecuteQueryAndGetResult() "));
        return false;
    }

    return false;
}

template <typename T>
T ExtractColumnValue(const mysqlx::Row& row, const uint32 index)
{
    // 범위 초과 시 예외 던지는거 같으니 따로 처리는 x
    return row[index].get<T>();
}

template <typename T, class... Args>
bool DBManager::FetchResults(mysqlx::SqlResult& sqlResult, TArray<T>& outResult)
{
    uint32 count = sqlResult.count();
    outResult.Reserve(count);

    for (const auto& row : sqlResult)
    {
        uint32 index = 0;
        try
        {
            T object{ ExtractColumnValue<Args>(row, index++)... };
            outResult.Emplace(std::move(object));
        }
        catch (...)
        {
            UE_LOG(LogTemp, Warning, TEXT("타입 캐스트 실패."));
            return false;
        }
    }   

    return true;
}