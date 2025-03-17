// Fill out your copyright notice in the Description page of Project Settings.


#include "StageInfoSubsystem.h"
#include "SpawnGroupInfoSubsystem.h"
#include "GameInfoManagementSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

bool FStageInfo::IsValid()
{
    if (_name.IsEmpty() == true)
    {
        return false;
    }

    for (const auto& key : _spawnGroupKeyArray)
    {
        if (key.IsValid() == false)
        {
            return false;
        }
    }

	return true;
}

int32 FStageInfo::GetMemorySize() const
{
    return  sizeof(FStageInfo)
        + _name.GetAllocatedSize()
        + _spawnGroupKeyArray.GetAllocatedSize();
}

void UStageInfoSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
    collection.InitializeDependency<USpawnGroupInfoSubsystem>();

	Super::Initialize(collection);

    ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\StageInfo.json"));
}

void UStageInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UStageInfoSubsystem::ValidateInfoData(const FString& filePath)
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

        const StageKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("StageKey : [%d] is not valid"), key.GetKey());
            return;
        }

        if (_stageInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("StageKey : [%d] is not valid"), key.GetKey());
            return;
        }

        TSharedPtr<StageInfo> stageInfo = MakeInfoByJsonObject(jsonObject);
        if (stageInfo == nullptr || stageInfo->IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 stageInfo 입니다. StageKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(stageInfo, EInfoLifeSpan::Manual);
        _stageInfoMap.Emplace(key, stageInfo);
    }
}

void UStageInfoSubsystem::LoadDynamicInfo(const StageKey key)   
{
    TWeakPtr<StageInfo>* stageInfoWeak = _stageInfoMap.Find(key);
    ensureMsgfDebug(stageInfoWeak != nullptr, TEXT("InfoMap에 해당 StageKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const StageKey stageKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (stageKey != key)
        {
            continue;
        }

        TSharedPtr<StageInfo> stageInfo = MakeInfoByJsonObject(jsonObject);
        *stageInfoWeak = stageInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(stageInfo, EInfoLifeSpan::Stage);

        return;
    }
}

DynamicStageInfo UStageInfoSubsystem::GetInfo(const StageKey key)
{
    TWeakPtr<StageInfo>* info = _stageInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("QuestKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("QuestKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicStageInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load StageInfo( StageKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicStageInfo(info->Pin(), key);
}

TSharedPtr<StageInfo> UStageInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<StageInfo> stageInfo = MakeShared<StageInfo>();
    stageInfo->_name = jsonObject->GetStringField(TEXT("Name"));

    FString spawnGroupKeyArrayStr = jsonObject->GetStringField(TEXT("SpawnGroupKey"));

    TArray<FString> spawnGroupKeyTokens;
    spawnGroupKeyArrayStr.ParseIntoArray(spawnGroupKeyTokens, TEXT(" "), true);

    for (const FString& spawnGroupKeyToken : spawnGroupKeyTokens)
    {
        int32 keyRaw = FCString::Atoi(*spawnGroupKeyToken);

        stageInfo->_spawnGroupKeyArray.Emplace(keyRaw);
        stageInfo->_totalSpawnCount += GetInfoSubsystem(USpawnGroupInfoSubsystem)->GetInfo(keyRaw)->_spawnDataArray.Num();
    }

    FString villagePosStr = jsonObject->GetStringField(TEXT("VillagePos"));

    if (villagePosStr.IsEmpty() == true)
    {
        stageInfo->_villagePos = FVector::ZeroVector;
        return stageInfo;
    }

    TArray<FString> villagePosTokens;
    villagePosStr.ParseIntoArray(villagePosTokens, TEXT("_"), true);

    if (villagePosTokens.Num() != 3)
    {
        ensureMsgf(false, TEXT("비정상적인 VillagePos 데이터 입니다 : [%s]"), *villagePosStr);
        return nullptr;
    }

    stageInfo->_villagePos = FVector(FCString::Atof(*villagePosTokens[0]), FCString::Atof(*villagePosTokens[1]), FCString::Atof(*villagePosTokens[2]));

    return stageInfo;
}