// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestInfoSubsystem.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

#include "GameInfoManagementSubsystem.h"

template<> constexpr int32 GetEnumSize<EConditionType>() { return static_cast<int32>(EConditionType::Count); }
DefineConvertEnumAndString(EConditionType, "KillMonster", "GetItem", "UseItem", "Count")
static_assert(GetEnumSize<EConditionType>() == 3, "Enum변경 시 대응이 필요합니다.");

template<> constexpr int32 GetEnumSize<ERewardType>() { return static_cast<int32>(ERewardType::Count); }
DefineConvertEnumAndString(ERewardType, "AcceptQuest", "GiveItem", "Count")
static_assert(GetEnumSize<ERewardType>() == 2, "Enum변경 시 대응이 필요합니다.");

bool FClearCondition::IsValid() const
{
    if (_type == EConditionType::Count
        || _conditionKeyRaw < 0
        || _clearCount < 0)
    {
        return false;
    }

    return true;
}


bool FQuestReward::IsValid() const
{
    if (_type == ERewardType::Count
        || _rewardKeyRaw < 0
        || _rewardCount < 0)
    {
        return false;
    }

    return true;
}


bool FQuestInfo::IsValid() const
{
    if (_name.IsEmpty() 
        || _desc.IsEmpty() 
        || _prevKey < 0
        || _clearCondition.IsValid() == false)
    {
        return false;
    }

	return true;
}

int32 FQuestInfo::GetMemorySize() const
{
    return  sizeof(FQuestInfo)
        + _questRewardArray.GetAllocatedSize()
        + _name.GetAllocatedSize()
        + _desc.GetAllocatedSize();
}

void UQuestInfoSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	ValidateInfoData(FPaths::ProjectContentDir() + FString("GameData\\GameInfo\\QuestInfo.json"));
}

void UQuestInfoSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UQuestInfoSubsystem::ValidateInfoData(const FString& filePath)
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

        const QuestKey key = jsonObject->GetIntegerField(TEXT("Key"));
        if (key.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 키 입니다 QuestKey : [%d]"), key.GetKey());
            return;
        }

        if (_questInfoMap.Find(key) != nullptr)
        {
            ensureMsgf(false, TEXT("중복된 키 값이 존재합니다 QuestKey : [%d]"), key.GetKey());
            return;
        }

        TSharedPtr<QuestInfo> questInfo = MakeInfoByJsonObject(jsonObject);
        if (questInfo == nullptr || questInfo.IsValid() == false)
        {
            ensureMsgf(false, TEXT("유효하지 않은 QuestInfo 입니다. QuestKey : [%d]"), key.GetKey());
            return;
        }

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(questInfo, EInfoLifeSpan::Manual);
        _questInfoMap.Emplace(key, questInfo);
    }
}

void UQuestInfoSubsystem::LoadDynamicInfo(const QuestKey key)
{
    TWeakPtr<QuestInfo>* questInfoWeak = _questInfoMap.Find(key);
    ensureMsgfDebug(questInfoWeak != nullptr, TEXT("InfoMap에 해당 QuestKey : [%d]가 존재하지 않습니다."), key.GetKey());

    FString jsonString;

    FFileHelper::LoadFileToString(jsonString, *_infoFilePath);
    TSharedRef<TJsonReader<>> jsonReader = TJsonReaderFactory<>::Create(jsonString);

    TArray<TSharedPtr<FJsonValue>> jsonArray;
    FJsonSerializer::Deserialize(jsonReader, jsonArray);

    for (const TSharedPtr<FJsonValue>& jsonValue : jsonArray)
    {
        TSharedPtr<FJsonObject> jsonObject = jsonValue->AsObject();

        const QuestKey questKey = jsonObject->GetIntegerField(TEXT("Key"));
        if (questKey != key)
        {
            continue;
        }

        TSharedPtr<QuestInfo> questInfo = MakeInfoByJsonObject(jsonObject);
        *questInfoWeak = questInfo;

        GetInfoSubsystem(UGameInfoManagementSubsystem)->RegisterDynamicInfo(questInfo, EInfoLifeSpan::Stage);

        return;
    }
}

DynamicQuestInfo UQuestInfoSubsystem::GetInfo(const QuestKey key)
{
    TWeakPtr<QuestInfo>* info = _questInfoMap.Find(key);
    if (info == nullptr)
    {
        ensureMsgf(false, TEXT("QuestKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다."), key.GetKey());
        UE_LOG(LogTemp, Error, TEXT("QuestKey : [%d]가 InfoMap에 없습니다. 비정상적인 상황입니다. nullptr을 반환합니다."), key.GetKey());

        return DynamicQuestInfo(nullptr, key);
    }

    if (info->IsValid() == false)
    {
        CheckTime(loadTime, LoadDynamicInfo(key));

        UE_LOG(LogTemp, Display, TEXT("Load QuestInfo( QuestKey : [%d] ), Info Size : [%d] byte. Loading Time : [%f] seconds")
            , key.GetKey(), info->Pin()->GetMemorySize(), loadTime);
    }

    return DynamicQuestInfo(info->Pin(), key);
}

TSharedPtr<QuestInfo> UQuestInfoSubsystem::MakeInfoByJsonObject(const TSharedPtr<FJsonObject>& jsonObject)
{
    TSharedPtr<QuestInfo> questInfo = MakeShared<QuestInfo>();
    {
        FString clearConditionStr = jsonObject->GetStringField(TEXT("ClearCondition"));

        TArray<FString> clearConditionTokens;
        clearConditionStr.ParseIntoArray(clearConditionTokens, TEXT(" "), true);

        int32 conditionTypeRaw = ConvertStringToEnum<EConditionType>(TCHAR_TO_UTF8(*clearConditionTokens[0]));
        if (conditionTypeRaw < 0 || conditionTypeRaw >= GetEnumSize<EConditionType>())
        {
            return nullptr;
        }
        
        ClearCondition clearCondition(static_cast<EConditionType>(conditionTypeRaw), 
            FCString::Atoi(*clearConditionTokens[1]), 
            FCString::Atoi(*clearConditionTokens[2]));

        if (clearCondition.IsValid() == false)
        {
            ensureMsgf(false, TEXT("비정상적인 ClearCondition 데이터 입니다 : [%s]"), *clearConditionStr);
            return nullptr;
        }

        questInfo->_clearCondition = std::move(clearCondition);
    }

    {
        FString rewardArrayStr = jsonObject->GetStringField(TEXT("Reward"));

        TArray<FString> rewardArrayTokens;
        rewardArrayStr.ParseIntoArray(rewardArrayTokens, TEXT(" "), true);

        for (const FString& rewardStr : rewardArrayTokens)
        {
            TArray<FString> rewardTokens;
            rewardStr.ParseIntoArray(rewardTokens, TEXT("_"), true);

            int32 rewardTypeRaw = ConvertStringToEnum<ERewardType>(TCHAR_TO_UTF8(*rewardTokens[0]));
            if (rewardTypeRaw < 0 || rewardTypeRaw >= GetEnumSize<ERewardType>())
            {
                return nullptr;
            }
                    
            ERewardType rewardType = static_cast<ERewardType>(rewardTypeRaw);
            int32 rewardKey = -1;
            int32 rewardCount = -1;

            switch (rewardType)
            {
            case ERewardType::AcceptQuest:
            {
                rewardKey = FCString::Atoi(*rewardTokens[1]);
                rewardCount = 1;

                break;
            }
            case ERewardType::GiveItem:
            {
                rewardKey = FCString::Atoi(*rewardTokens[1]);
                rewardCount = FCString::Atoi(*rewardTokens[2]);

                break;
            }
            default: break;
            }

            QuestReward questReward(rewardType, rewardKey, rewardCount);
            if (questReward.IsValid() == false)
            {
                ensureMsgf(false, TEXT("비정상적인 QuestReward 데이터 입니다 : [%s]"), *rewardStr);
                return nullptr;
            }

            questInfo->_questRewardArray.Emplace(std::move(questReward));
        }
    }

    questInfo->_name = jsonObject->GetStringField(TEXT("Name"));
    questInfo->_desc = jsonObject->GetStringField(TEXT("Desc"));
    questInfo->_qusetType = static_cast<EQuestType>(jsonObject->GetIntegerField(TEXT("QuestType")));
    questInfo->_prevKey = jsonObject->GetIntegerField(TEXT("PrevKey"));

    return questInfo;
}