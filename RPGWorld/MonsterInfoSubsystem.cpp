// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterInfoSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "BaseMonster.h"
#include "GameInfoManagementSubsystem.h"

bool FMonsterInfo::IsValid()
{
    if ( _monsterClass.IsNull() == true
        || _dropGroupKey.IsValid() == false
        || _name.IsEmpty()      == true
        || _aiMoveRadius        < 0.f
        || _health              <= 0
        || _attackDamage        <= 0
        || _level               <= 0 )
    {
        return false;
    }

	return true;
}

int32 FMonsterInfo::GetMemorySize() const
{
    return  sizeof(FMonsterInfo)
        + _name.GetAllocatedSize();
}

void UMonsterInfoSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
    Super::Initialize(collection);

    ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\MonsterInfo.json"));
}

void UMonsterInfoSubsystem::Deinitialize()
{
    Super::Deinitialize();
}

void UMonsterInfoSubsystem::ValidateInfoData(const FString& filePath)
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
   
        const MonsterKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 키 입니다 MonsterKey : [%d]"), key.GetKey());
            return;
        }

        if (_monsterInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("중복된 키 값이 존재합니다 MonsterKey : [%d]"), key.GetKey());
            return;
        }


        TSharedPtr<MonsterInfo> monsterInfo = MakeInfoByJsonObject(jsonObject);
        if (monsterInfo == nullptr || monsterInfo.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 MonsterInfo 입니다. MonsterKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(monsterInfo, EInfoLifeSpan::Manual);
        _monsterInfoMap.Emplace(key, monsterInfo);
    }

}

void UMonsterInfoSubsystem::LoadDynamicInfo(const MonsterKey key)
{
    TWeakPtr<MonsterInfo>* monsterInfoWeak = _monsterInfoMap.Find(key);
    ensureMsgfDebug(monsterInfoWeak != nullptr, TEXT("InfoMap에 해당 QuestKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const MonsterKey monsterKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (monsterKey != key)
        {
            continue;
        }

        TSharedPtr<MonsterInfo> monsterInfo = MakeInfoByJsonObject(jsonObject);
        *monsterInfoWeak = monsterInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(monsterInfo, EInfoLifeSpan::Stage);

        return;
    }
}

DynamicMonsterInfo UMonsterInfoSubsystem::GetInfo(const MonsterKey key)
{
    TWeakPtr<MonsterInfo>* info = _monsterInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("MonsterKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("MonsterKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicMonsterInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load MonsterInfo( MonsterKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicMonsterInfo(info->Pin(), key);
}

TSharedPtr<MonsterInfo> UMonsterInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<MonsterInfo> monsterInfo = MakeShared<MonsterInfo>();
    monsterInfo->_dropGroupKey.SetKeyRawValue(jsonObject->GetNumberField(TEXT("DropGroupKey")));
    monsterInfo->_name              = jsonObject->GetStringField(TEXT("Name"));
    monsterInfo->_level             = jsonObject->GetIntegerField(TEXT("Level"));
    monsterInfo->_health            = jsonObject->GetIntegerField(TEXT("Health"));
    monsterInfo->_attackDamage      = jsonObject->GetIntegerField(TEXT("AttackDamage"));
    monsterInfo->_aiMoveRadius      = jsonObject->GetNumberField(TEXT("AIMoveRadius"));
    monsterInfo->_monsterClass      = jsonObject->GetStringField(TEXT("AssetPath"));

    return monsterInfo;
}
