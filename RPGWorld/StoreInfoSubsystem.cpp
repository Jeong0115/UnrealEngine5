#include "StoreInfoSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "GameInfoManagementSubsystem.h"

constexpr int32 ITEM_PRICE_TOKEN_COUNT          = 2;
constexpr int32 TOKEN_STORE_ITEM_KEY_INDEX      = 0;
constexpr int32 TOKEN_STORE_ITEM_PRICE_INDEX    = 1;

bool FItemPrice::IsValid() const
{
    if (_itemKey.IsValid() == false
        || _price < 0)
    {
        return false;
    }

    return true;
}

bool FStoreInfo::IsValid() const
{
    if (_itemPriceArray.IsEmpty() == true)
    {
        return false;
    }

	return true;
}

int32 FStoreInfo::GetMemorySize() const
{
    return  sizeof(FStoreInfo)
        + _itemPriceArray.GetAllocatedSize();
}

void UStoreInfoSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\StoreInfo.json"));
}

void UStoreInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UStoreInfoSubsystem::ValidateInfoData(const FString& filePath)
{
    _infoFilePath = filePath;

    FString jsonString;
    if (FFileHelper::LoadFileToString(jsonString, *_infoFilePath) == false)
    {
        ensureMsgf(false, TEXT("File Load Failed \nFilePath : [%s]"), *_infoFilePath);
        return;
    }

    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);
    TArray<TSharedPtr<FJsonValue>> jsonArray;

    if (FJsonSerializer::Deserialize(jsonReader, jsonArray) == false || jsonArray.Num() == 0)
    {
        ensureMsgf(false, TEXT("Failed to parse JSON \nFilePath : [%s]"), *_infoFilePath);
        return;
    }

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        if (jsonValue.IsValid() == false)
        {
            ensureMsgf(false, TEXT("Failed to parse JSON \nFilePath : [%s]"), *_infoFilePath);
            return;
        }

        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const StoreKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 키 입니다 StoreKey : [%d]"), key.GetKey());
            return;
        }

        if (_storeInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("중복된 키 값이 존재합니다 StoreKey : [%d]"), key.GetKey());
            return;
        }

        TSharedPtr<StoreInfo> storeInfo = MakeInfoByJsonObject(jsonObject);
        if (storeInfo == nullptr || storeInfo.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 StoreInfo 입니다. StoreKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(storeInfo, EInfoLifeSpan::Manual);
        _storeInfoMap.Emplace(key, storeInfo);
    }
}

void UStoreInfoSubsystem::LoadDynamicInfo(const StoreKey key)
{
    TWeakPtr<StoreInfo>* storeInfoWeak = _storeInfoMap.Find(key);
    ensureMsgfDebug(storeInfoWeak != nullptr, TEXT("InfoMap에 해당 StoreKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const StoreKey storeKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (storeKey != key)
        {
            continue;
        }

        TSharedPtr<StoreInfo> storeInfo = MakeInfoByJsonObject(jsonObject);
        *storeInfoWeak = storeInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(storeInfo, EInfoLifeSpan::Stage);

        return;
    }
}

DynamicStoreInfo UStoreInfoSubsystem::GetInfo(const StoreKey key)
{
    TWeakPtr<StoreInfo>* info = _storeInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("StoreKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("StoreKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicStoreInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load StoreKey( StoreKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicStoreInfo(info->Pin(), key);
}

TSharedPtr<StoreInfo> UStoreInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<StoreInfo> storeInfo = MakeShared<StoreInfo>();
    {
        FString itemPirceArrayStr = jsonObject->GetStringField(TEXT("ItemPrices"));

        TArray<FString> itemPirceArrayTokens;
        itemPirceArrayStr.ParseIntoArray(itemPirceArrayTokens, TEXT(" "), true);

        for (const FString& itemPirceStr : itemPirceArrayTokens)
        {
            TArray<FString> itemPirceTokens;
            itemPirceStr.ParseIntoArray(itemPirceTokens, TEXT("_"), true);

            if (itemPirceTokens.Num() != ITEM_PRICE_TOKEN_COUNT)
            {
                ensureMsgf(false, TEXT("ItemPrice 토큰 개수가 비정상 입니다: [%d]"), itemPirceTokens.Num());
                return nullptr;
            }

            ItemPrice itemPrice
            (
                FCString::Atoi(*itemPirceTokens[TOKEN_STORE_ITEM_KEY_INDEX]), 
                FCString::Atoi(*itemPirceTokens[TOKEN_STORE_ITEM_PRICE_INDEX])
            );

            if (itemPrice.IsValid() == false)
            {
                ensureMsgf(false, TEXT("비정상적인 ItemPrice 데이터 입니다 : [%s]"), *itemPirceStr);
                return nullptr;
            }

            storeInfo->_itemPriceArray.Emplace(std::move(itemPrice));
        }
    }

    return storeInfo;
}


