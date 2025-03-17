// Fill out your copyright notice in the Description page of Project Settings.


#include "SpawnGroupInfoSubsystem.h"
#include "GameInfoManagementSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

constexpr int32 SPAWN_POSITION_TOKEN_COUNT  = 3;
constexpr int32 SPAWN_ROTATION_TOKEN_COUNT  = SPAWN_POSITION_TOKEN_COUNT;
constexpr int32 SPAWN_SCALE_TOKEN_COUNT     = SPAWN_POSITION_TOKEN_COUNT;

constexpr int32 TOKEN_KEY_INDEX             = 0;
constexpr int32 TOKEN_POSITION_X_INDEX      = 0;
constexpr int32 TOKEN_POSITION_Y_INDEX      = 1;
constexpr int32 TOKEN_POSITION_Z_INDEX      = 2;
constexpr int32 TOKEN_PITCH_INDEX           = 0;
constexpr int32 TOKEN_YAW_INDEX             = 1;    
constexpr int32 TOKEN_ROLL_INDEX            = 2;
constexpr int32 TOKEN_SCALE_X_INDEX         = TOKEN_POSITION_X_INDEX;
constexpr int32 TOKEN_SCALE_Y_INDEX         = TOKEN_POSITION_Y_INDEX;
constexpr int32 TOKEN_SCALE_Z_INDEX         = TOKEN_POSITION_Z_INDEX;

bool FSpawnMonsterData::IsValid() const
{
    if (_monsterKey.IsValid() == false 
        || _spawnScale.X <= 0.f 
        || _spawnScale.Y <= 0.f 
        || _spawnScale.Z <= 0.f)
    {
        return false;
    }

    return true;
}

FSpawnGroupInfo::FSpawnGroupInfo()
    : _spawnDuration(0.f)
{
}

bool FSpawnGroupInfo::IsValid() const
{
    if (_spawnDuration < 0.f)
    {
        return false;
    }

    return true;
}

int32 FSpawnGroupInfo::GetMemorySize() const
{
    return  sizeof(FSpawnGroupInfo)
        + _spawnDataArray.GetAllocatedSize();
}

void USpawnGroupInfoSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\SpawnGroupInfo.json"));
}

void USpawnGroupInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void USpawnGroupInfoSubsystem::ValidateInfoData(const FString& filePath)
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

        const SpawnGroupKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 키 입니다 SpawnGroupKey : [%d]"), key.GetKey());
            return;
        }

        if (_spawnGroupInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("중복된 키 값이 존재합니다 SpawnGroupKey : [%d]"), key.GetKey());
            return;
        }
        
        TSharedPtr<SpawnGroupInfo> spawnGroupInfo = MakeInfoByJsonObject(jsonObject);
        if (spawnGroupInfo == nullptr || spawnGroupInfo.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 SpawnGroupInfo 입니다. SpawnGroupKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(spawnGroupInfo, EInfoLifeSpan::Manual);
        _spawnGroupInfoMap.Emplace(key, spawnGroupInfo);
    }
}

void USpawnGroupInfoSubsystem::LoadDynamicInfo(const SpawnGroupKey key)
{
    TWeakPtr<SpawnGroupInfo>* spawnGroupInfoWeak = _spawnGroupInfoMap.Find(key);
    ensureMsgfDebug(spawnGroupInfoWeak != nullptr, TEXT("InfoMap에 해당 QuestKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const SpawnGroupKey spawnGroupKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (spawnGroupKey != key)
        {
            continue;
        }

        TSharedPtr<SpawnGroupInfo> spawnGroupInfo = MakeInfoByJsonObject(jsonObject);
        *spawnGroupInfoWeak = spawnGroupInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(spawnGroupInfo, EInfoLifeSpan::Stage);

        return;
    }
}


DynamicSpawnGroupInfo USpawnGroupInfoSubsystem::GetInfo(const SpawnGroupKey key)
{
    TWeakPtr<SpawnGroupInfo>* info = _spawnGroupInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("SpawnGroupKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("SpawnGroupKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicSpawnGroupInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load SpawnGroupInfo( SpawnGroupKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicSpawnGroupInfo(info->Pin(), key);
}

TSharedPtr<SpawnGroupInfo> USpawnGroupInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<SpawnGroupInfo> spawnGroupInfo = MakeShared<SpawnGroupInfo>();

    int32 spawnCount = jsonObject->GetIntegerField(TEXT("SpawnCount"));
    if (spawnCount < 0)
    {
        ensureMsgf(false, TEXT("스폰 카운트가 0보다 작을 수 있나요? : [%d]"), spawnCount);
        return nullptr;
    }

    spawnGroupInfo->_spawnDuration = jsonObject->GetNumberField(TEXT("SpawnDuration"));

    FString monsterKeyString    = jsonObject->GetStringField(TEXT("MonsterKey"));
    FString spawnPositionString = jsonObject->GetStringField(TEXT("SpawnPosition"));
    FString spawnRotationString = jsonObject->GetStringField(TEXT("SpawnRotation"));
    FString spawnScaleString = jsonObject->GetStringField(TEXT("SpawnScale"));

    TArray<FString> monsterKeyTokens;
    TArray<FString> spawnPositionTokens;
    TArray<FString> spawnRotationTokens;
    TArray<FString> spawnScaleTokens;

    monsterKeyString.ParseIntoArray(monsterKeyTokens, TEXT(" "), true);
    spawnPositionString.ParseIntoArray(spawnPositionTokens, TEXT(" "), true);
    spawnRotationString.ParseIntoArray(spawnRotationTokens, TEXT(" "), true);
    spawnScaleString.ParseIntoArray(spawnScaleTokens, TEXT(" "), true);

    if (monsterKeyTokens.Num()          != spawnCount
        || spawnPositionTokens.Num()    != spawnCount
        || spawnRotationTokens.Num()    != spawnCount
        || spawnScaleTokens.Num()       != spawnCount)
    {
        ensureMsgf(false,
            TEXT("스폰 카운트와 스폰 데이터의 개수가 일치하지 않습니다. SpawnCount: [%d], MonsterKeySize: [%d], PositionSize: [%d], RotationSize: [%d], ScaleSize: [%d]"),
            spawnCount,
            monsterKeyTokens.Num(),
            spawnPositionTokens.Num(),
            spawnRotationTokens.Num(),
            spawnScaleTokens.Num());

        return nullptr;
    }

    spawnGroupInfo->_spawnDataArray.Reserve(spawnCount);

    for (int32 index = 0; index < spawnCount; ++index)
    {
        TArray<FString> spawnPositionToken;
        TArray<FString> spawnRotationToken;
        TArray<FString> spawnScaleToken;

        spawnPositionTokens[index].ParseIntoArray(spawnPositionToken, TEXT("_"), true);
        spawnRotationTokens[index].ParseIntoArray(spawnRotationToken, TEXT("_"), true);
        spawnScaleTokens[index].ParseIntoArray(spawnScaleToken, TEXT("_"), true);

        if (spawnPositionToken.Num() != SPAWN_POSITION_TOKEN_COUNT)
        {
            ensureMsgf(false, TEXT("비정상적인 SpawnPosition 데이터 입니다 : [%s]"), *spawnPositionTokens[index]);
            return nullptr;
        }

        if (spawnRotationToken.Num() != SPAWN_ROTATION_TOKEN_COUNT)
        {
            ensureMsgf(false, TEXT("비정상적인 SpawnRotation 데이터 입니다 : [%s]"), *spawnRotationTokens[index]);
            return nullptr;
        }

        if (spawnScaleToken.Num() != SPAWN_SCALE_TOKEN_COUNT)
        {
            ensureMsgf(false, TEXT("비정상적인 SpawnScale 데이터 입니다 : [%s]"), *spawnScaleTokens[index]);
            return nullptr;
        }

        MonsterKey key = FCString::Atoi(*monsterKeyTokens[index]);

        FVector loaction(
            FCString::Atof(*spawnPositionToken[TOKEN_POSITION_X_INDEX]),
            FCString::Atof(*spawnPositionToken[TOKEN_POSITION_Y_INDEX]),
            FCString::Atof(*spawnPositionToken[TOKEN_POSITION_Z_INDEX]));

        FRotator rotation(
            FCString::Atof(*spawnRotationToken[TOKEN_PITCH_INDEX]),
            FCString::Atof(*spawnRotationToken[TOKEN_YAW_INDEX]),
            FCString::Atof(*spawnRotationToken[TOKEN_ROLL_INDEX]));

        FVector scale(
            FCString::Atof(*spawnScaleToken[TOKEN_SCALE_X_INDEX]),
            FCString::Atof(*spawnScaleToken[TOKEN_SCALE_Y_INDEX]),
            FCString::Atof(*spawnScaleToken[TOKEN_SCALE_Z_INDEX]));

        FSpawnMonsterData spawnMonsterData(key, loaction, rotation, scale);

        if (spawnMonsterData.IsValid() == false)
        {
            ensureMsgf(false, TEXT("비정상적인 SpawnMonsterData 데이터입니다 MonsterKey : [%d]"), key.GetKey());
            return nullptr;
        }

        spawnGroupInfo->_spawnDataArray.Emplace(spawnMonsterData);
    }

    return spawnGroupInfo;
}
