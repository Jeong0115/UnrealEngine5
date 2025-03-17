#include "ItemInfoSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "GameInfoManagementSubsystem.h"

bool FRPGItemInfo::IsValid() const
{
    if (_itemTexture.IsNull() == true
        || _name.IsEmpty() == true
        || _desc.IsEmpty() == true
        || _maxStack <= 0)
    {
        return false;
    }

	return true;
}

int32 FRPGItemInfo::GetMemorySize() const
{
    return  sizeof(FRPGItemInfo)
        + _name.GetAllocatedSize()
        + _desc.GetAllocatedSize()
        + _useFunc.GetAllocatedSize();
}

void UItemInfoSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\ItemInfo.json"));
}

void UItemInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UItemInfoSubsystem::ValidateInfoData(const FString& filePath)
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

        const ItemKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 키 입니다 ItemKey : [%d]"), key.GetKey());
            return;
        }

        if (_itemInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("중복된 키 값이 존재합니다 ItemKey : [%d]"), key.GetKey());
            return;
        }

        TSharedPtr<RPGItemInfo> itemInfo = MakeInfoByJsonObject(jsonObject);
        if (itemInfo == nullptr || itemInfo->IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 ItemInfo 입니다. ItemKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(itemInfo, EInfoLifeSpan::Manual);
        _itemInfoMap.Emplace(key, itemInfo);
    }
}

void UItemInfoSubsystem::LoadDynamicInfo(const ItemKey key)
{
    TWeakPtr<RPGItemInfo>* itemInfoWeak = _itemInfoMap.Find(key);
    ensureMsgfDebug(itemInfoWeak != nullptr, TEXT("InfoMap에 해당 ItemKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const ItemKey itemKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (itemKey != key)
        {
            continue;
        }

        TSharedPtr<RPGItemInfo> itemInfo = MakeInfoByJsonObject(jsonObject);
        *itemInfoWeak = itemInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(itemInfo, EInfoLifeSpan::Stage);

        return;
    }
}

DynamicItemInfo UItemInfoSubsystem::GetInfo(const ItemKey key)
{
    TWeakPtr<RPGItemInfo>* info = _itemInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("ItemKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("ItemKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicItemInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load RPGItemInfo( ItemKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicItemInfo(info->Pin(), key);
}

TSharedPtr<RPGItemInfo> UItemInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<RPGItemInfo> itemInfo = MakeShared<RPGItemInfo>();
    itemInfo->_name         = jsonObject->GetStringField(TEXT("Name"));
    itemInfo->_desc         = jsonObject->GetStringField(TEXT("Desc"));
    itemInfo->_itemTexture  = jsonObject->GetStringField(TEXT("IconPath"));
    itemInfo->_useFunc      = jsonObject->GetStringField(TEXT("UseFunc"));
    itemInfo->_maxStack     = jsonObject->GetNumberField(TEXT("MaxStack"));

    if (itemInfo->_useFunc.IsEmpty() == true)
    {
        itemInfo->_bCanUseInven = false;
        itemInfo->_bUseEffect   = false;
    }
    else
    {
        itemInfo->_bCanUseInven = jsonObject->GetBoolField(TEXT("CanUseInven"));
        itemInfo->_bUseEffect   = true;
    }

    return itemInfo;
}
