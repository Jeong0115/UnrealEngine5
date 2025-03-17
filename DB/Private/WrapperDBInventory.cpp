#include "WrapperDBInventory.h"

#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

#include "DBManager.h"

namespace WrapperDB
{
    constexpr const char* uspSelectUserInventory    = "CALL SelectUserInventory(?)";
    constexpr const char* uspSaveInventory          = "CALL SaveInventory(?, ?)";
    constexpr const char* uspRewardItemInventory    = "CALL RewardItemInventory(?, ?)";
    constexpr const char* uspPurchaseItemInventory  = "CALL PurchaseItemInventory(?, ?)";
    constexpr const char* uspSortInventory          = "CALL SortInventory(?, ?)";
    constexpr const char* uspUseItemInventory       = "CALL UseItemInventory(?, ?, ?, ?)";

    void CreateFStringFromInvenTableArray(FString& outString, const TArray<FInventoryTable>& inventoryTableArray);

	ErrNo SaveInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId)
	{
		mysqlx::SqlResult result;

        std::unique_ptr<mysqlx::Session>& session = DBManager::GetInstance()->GetSession();
        try 
        {
            FString invenTabelString;
            CreateFStringFromInvenTableArray(invenTabelString, inventoryTableArray);

            auto stmt = session->sql(uspSaveInventory);
            stmt.bind(userId, TCHAR_TO_UTF8(*invenTabelString));

            auto res = stmt.execute();

            return ErrNo(0);
        }
        catch (...)
        {
            HandleDBException();
            return errFailedExecuteQuery;
        }

		return ErrNo(0);
	}

    ErrNo LoadInventory(TArray<FInventoryTable>& outInventoryTableArray, const int32 userId)
    {
        mysqlx::SqlResult result;

        ErrNo errNo(0);

        DBManager* db = DBManager::GetInstance();

        if (db->ExecuteQueryAndGetResult(uspSelectUserInventory, result, userId) == false)
        {
            errNo = errFailedExecuteQuery;
            return errNo;
        }

        if (db->FetchResults<FInventoryTable, InventoryTableArgs>(result, outInventoryTableArray) == false)
        {
            errNo = errFailedApplyQueryResult;
            return errNo;
        }

        return ErrNo(0);
    }

    ErrNo RewardItemToInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId)
    {

        DBManager* instance = DBManager::GetInstance();
        std::unique_ptr<mysqlx::Session>& session = instance->GetSession();
        try
        {
            FString invenTabelString;
            CreateFStringFromInvenTableArray(invenTabelString, inventoryTableArray);

            auto stmt = session->sql(uspRewardItemInventory);
            stmt.bind(userId, TCHAR_TO_UTF8(*invenTabelString));

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
            // 여기서 실패한거면 DB랑, 서버 쪽이랑 인벤토리 정보가 일치하지 않을 확률이 크다...
            return errFailedExecuteQuery;
        }

        return ErrNo(0);
    }

    ErrNo PurchaseItemToInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId)
    {
        DBManager* instance = DBManager::GetInstance();
        std::unique_ptr<mysqlx::Session>& session = instance->GetSession();
        try
        {
            FString invenTabelString;
            CreateFStringFromInvenTableArray(invenTabelString, inventoryTableArray);

            auto stmt = session->sql(uspPurchaseItemInventory);
            stmt.bind(userId, TCHAR_TO_UTF8(*invenTabelString));

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
            // 여기서 실패한거면 DB랑, 서버 쪽이랑 인벤토리 정보가 일치하지 않을 확률이 크다...
            return errFailedExecuteQuery;
        }

        return ErrNo(0);
    }

    ErrNo SortInventory(const TArray<FInventoryTable>& inventoryTableArray, const int32 userId)
    {
        DBManager* instance = DBManager::GetInstance();
        std::unique_ptr<mysqlx::Session>& session = instance->GetSession();
        try
        {
            FString invenTabelString;
            CreateFStringFromInvenTableArray(invenTabelString, inventoryTableArray);

            auto stmt = session->sql(uspSortInventory);
            stmt.bind(userId, TCHAR_TO_UTF8(*invenTabelString));
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
            // 여기서 실패한거면 DB랑, 서버 쪽이랑 인벤토리 정보가 일치하지 않을 확률이 크다...
            return errFailedExecuteQuery;
        }

        return ErrNo(0);
    }

    ErrNo UseItemInventory(const int32 userId, const int32 slotIndex, const int32 itemKeyRaw, const int32 itemCount)
    {
        std::wstring result;
        if (DBManager::GetInstance()->ExecuteQueryAndFetchResult(result, uspUseItemInventory, userId, slotIndex, itemKeyRaw, itemCount) == false)
        {
            FString str(result.c_str());
            UE_LOG(LogTemp, Warning, TEXT("%s"), *str);
            return errFailedExecuteQuery;
        }

        return ErrNo(0);
    }


    void CreateFStringFromInvenTableArray(FString& outString, const TArray<FInventoryTable>& inventoryTableArray)
    {
        TSharedPtr<FJsonObject> invenJson = MakeShareable(new FJsonObject);
        TArray<TSharedPtr<FJsonValue>> itemArray;

        for (const FInventoryTable& inventoryTable : inventoryTableArray)
        {
            TSharedPtr<FJsonObject> itemObject = MakeShareable(new FJsonObject);
            itemObject->SetNumberField("_slotIndex", inventoryTable._slotIndex);
            itemObject->SetNumberField("_itemKey", inventoryTable._itemKey);
            itemObject->SetNumberField("_itemCount", inventoryTable._itemCount);
            itemArray.Add(MakeShareable(new FJsonValueObject(itemObject)));
        }

        invenJson->SetArrayField("invenTable", itemArray);

        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&outString);
        FJsonSerializer::Serialize(invenJson.ToSharedRef(), Writer);
    }
}

