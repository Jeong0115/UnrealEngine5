#include "WrapperDBQuest.h"

#include "DBManager.h"

namespace WrapperDB
{
	constexpr const char* uspSelectUserQuest		= "CALL SelectUserQuest(?)";
	constexpr const char* uspUpdateQuestCondition	= "CALL UpdateQuestCondition(?, ?)";
	constexpr const char* uspAcceptQuest			= "CALL AcceptQuest(?, ?, ?, ?)";
	constexpr const char* uspClearQuest				= "CALL ClearQuest(?, ?)";

	void CreateFStringFromQuestTableArray(FString& outString, const TArray<FQuestTable>& inventoryTableArray);

	ErrNo SelectUserQuest(TArray<FQuestTable>& outQuestTable, int32 id)
	{
		mysqlx::SqlResult result;

		ErrNo errNo(0);

		if (DBManager::GetInstance()->ExecuteQueryAndGetResult(uspSelectUserQuest, result, id) == false)
		{
			errNo = errFailedExecuteQuery;
			return errNo;
		}

		if (DBManager::GetInstance()->FetchResults<FQuestTable, QuestTableArgs>(result, outQuestTable) == false)
		{
			errNo = errFailedApplyQueryResult;
			return errNo;
		}

		return ErrNo(0);
	}

	ErrNo UpdateQuestCondition(const TArray<FQuestTable>& questTableArray, const int32 userId)
	{
		DBManager* instance = DBManager::GetInstance();
		std::unique_ptr<mysqlx::Session>& session = instance->GetSession();
		try
		{
			FString questTableString;
			CreateFStringFromQuestTableArray(questTableString, questTableArray);

			auto stmt = session->sql(uspUpdateQuestCondition);
			stmt.bind(userId, TCHAR_TO_UTF8(*questTableString));

			auto result = stmt.execute();
			auto row = result.fetchOne();
			if (instance->IsSuccessQuery(row, 0) == false)
			{
				return errFailedExecuteQuery;
			}

			return ErrNo(0);
		}
		catch (...)
		{
			HandleDBException();

			return errFailedExecuteQuery;
		}

		return ErrNo(0);
	}

	ErrNo AcceptQuest(const int32 userId, const int32 questId, const int32 questType, const int32 clearCount)
	{
		std::wstring result;
		if (DBManager::GetInstance()->ExecuteQueryAndFetchResult(result, uspAcceptQuest, userId, questId, questType, clearCount) == false)
		{
			FString str(result.c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
			return errFailedExecuteQuery;
		}

		return ErrNo(0);
	}

	ErrNo ClearQuest(const int32 userId, const int32 questId)
	{
		std::wstring result;
		if (DBManager::GetInstance()->ExecuteQueryAndFetchResult(result, uspClearQuest, userId, questId) == false)
		{
			FString str(result.c_str());
			UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
			return errFailedExecuteQuery;
		}

		return ErrNo(0);
	}

	void CreateFStringFromQuestTableArray(FString& outString, const TArray<FQuestTable>& questTableArray)
	{
		TSharedPtr<FJsonObject> invenJson = MakeShareable(new FJsonObject);
		TArray<TSharedPtr<FJsonValue>> itemArray;

		for (const FQuestTable& questTable : questTableArray)
		{
			TSharedPtr<FJsonObject> itemObject = MakeShareable(new FJsonObject);
			itemObject->SetNumberField("_questId",			questTable._questId);
			itemObject->SetNumberField("_questType",		questTable._questType);
			itemObject->SetNumberField("_questState",		questTable._questState);
			itemObject->SetNumberField("_conditionCount",	questTable._conditionCount);
			itemArray.Add(MakeShareable(new FJsonValueObject(itemObject)));
		}

		invenJson->SetArrayField("questTable", itemArray);

		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&outString);
		FJsonSerializer::Serialize(invenJson.ToSharedRef(), Writer);
	}
}

