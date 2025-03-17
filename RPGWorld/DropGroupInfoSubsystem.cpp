#include "DropGroupInfoSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "GameInfoManagementSubsystem.h"

constexpr int32 DROP_ITEM_TOKEN_COUNT           = 3;

constexpr int32 TOKEN_ITEM_KEY_INDEX            = 0;
constexpr int32 TOKEN_ITEM_COUNT_INDEX          = 1;
constexpr int32 TOKEN_ITEM_PROBABILITY_INDEX    = 2;

bool FDropItem::IsValid() const
{
    if (_itemKey.IsValid()  == false
        || _itemCount       < 0
        || _probability     <= 0.f)
    {
        return false;
    }

    return true;
}

bool FDropGroupInfo::IsValid() const
{
    return true;
}

int32 FDropGroupInfo::GetMemorySize() const
{
    return  sizeof(FDropGroupInfo)
        + _dropItemArray.GetAllocatedSize();
}

void UDropGroupInfoSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
    Super::Initialize(collection);

    ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\DropGroupInfo.json"));
}

void UDropGroupInfoSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UDropGroupInfoSubsystem::ValidateInfoData(const FString& filePath)
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

        const DropGroupKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 키 입니다 DropGroupKey : [%d]"), key.GetKey());
            return;
        }

        if (_dropGroupInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("중복된 키 값이 존재합니다 DropGroupKey : [%d]"), key.GetKey());
            return;
        }

        TSharedPtr<DropGroupInfo> dropGroupInfo = MakeInfoByJsonObject(jsonObject);
        if (dropGroupInfo == nullptr || dropGroupInfo.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 DropGroupInfo 입니다. DropGroupKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(dropGroupInfo, EInfoLifeSpan::Manual);
        _dropGroupInfoMap.Emplace(key, dropGroupInfo);
    }
}

void UDropGroupInfoSubsystem::LoadDynamicInfo(const DropGroupKey key)
{
    TWeakPtr<DropGroupInfo>* dropGroupInfoWeak = _dropGroupInfoMap.Find(key);
    ensureMsgfDebug(dropGroupInfoWeak != nullptr, TEXT("InfoMap에 해당 DropGroupKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const DropGroupKey dropGroupKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (dropGroupKey != key)
        {
            continue;
        }

        TSharedPtr<DropGroupInfo> dropGroupInfo = MakeInfoByJsonObject(jsonObject);
        *dropGroupInfoWeak = dropGroupInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(dropGroupInfo, EInfoLifeSpan::Stage);

        return;
    }
}

DynamicDropGroupInfo UDropGroupInfoSubsystem::GetInfo(const DropGroupKey key)
{
    TWeakPtr<DropGroupInfo>* info = _dropGroupInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("DropGroupKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("DropGroupKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicDropGroupInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load DropGroupInfo( DropGroupKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicDropGroupInfo(info->Pin(), key);
}

TSharedPtr<DropGroupInfo> UDropGroupInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<DropGroupInfo> dropGroupInfo = MakeShared<DropGroupInfo>();

    FString dropItemArrayStr = jsonObject->GetStringField(TEXT("DropItem"));

    TArray<FString> dropItemArrayTokens;
    dropItemArrayStr.ParseIntoArray(dropItemArrayTokens, TEXT(" "), true);

    for (const FString& dropItemStr : dropItemArrayTokens)
    {
        TArray<FString> dropItemTokens;
        dropItemStr.ParseIntoArray(dropItemTokens, TEXT("_"), true);

        if (dropItemTokens.Num() != DROP_ITEM_TOKEN_COUNT)
        {
            ensureMsgf(false, TEXT("비정상적인 DropItem 데이터 입니다 : [%s]"), *dropItemStr);
            return nullptr;
        }

        int32 itemKey     = FCString::Atoi(*dropItemTokens[TOKEN_ITEM_KEY_INDEX]);
        int32 itemCount   = FCString::Atoi(*dropItemTokens[TOKEN_ITEM_COUNT_INDEX]);
        float probability = FCString::Atof(*dropItemTokens[TOKEN_ITEM_PROBABILITY_INDEX]);
        
        DropItem dropItem(itemKey, itemCount, probability);
        if (dropItem.IsValid() == false)
        {
            ensureMsgf(false, TEXT("비정상적인 DropItem 데이터 입니다 : [%s]"), *dropItemStr);
            return nullptr;
        }

        dropGroupInfo->_dropItemArray.Emplace(std::move(dropItem));
    }

    return dropGroupInfo;
}
