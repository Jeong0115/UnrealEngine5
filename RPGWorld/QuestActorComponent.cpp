
#include "QuestActorComponent.h"

#include "DB\Public\WrapperDB.h"
#include "DB\Public\WrapperDBQuest.h"

#include "BasePlayerController.h"
#include "InventoryActorComponent.h"
#include "QuestWidget.h"

void FQuestCondition::AddCurrentCount(const int32 count)
{
	_currentCount = FMath::Min(_currentCount + count, _clearCount);
}

void FQuestCondition::UpdateWidgetConditionCount() const
{
	if (_changeCountDelegate.IsBound() == true)
	{
		_changeCountDelegate.Execute(_currentCount);
	}
}

FString UQuestData::GetQuestName() const
{
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(_questKey);
		return questInfo->_name;
	}

	UE_LOG(LogTemp, Warning, TEXT("QuestName을 얻어오는 과정에서 실패하였습니다. 빈 문자열을 반환합니다."));
	return "";
}

FString UQuestData::GetQuestDesc() const
{
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(_questKey);
		return questInfo->_desc;
	}

	UE_LOG(LogTemp, Warning, TEXT("QuestDesc을 얻어오는 과정에서 실패하였습니다. 빈 문자열을 반환합니다."));
	return "";
}

int32 UQuestData::GetCurrentConditionCount() const
{
	return _questCondition->GetCurrentCount();
}

int32 UQuestData::GetClearConditionCount() const
{
	return _questCondition->GetClearCount();
}

UQuestActorComponent::UQuestActorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UQuestActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UQuestActorComponent::LoadDataFromDB()
{
#if WITH_SERVER_CODE

	TArray<FQuestTable> questTableArray;
	ErrNo errNo = WrapperDB::SelectUserQuest(questTableArray, GetUserID());
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("DB에서 Quest데이터를 얻어오는데 실패하였습니다. : [%s]"), *errNo.GetText());
		return;
	}

	Initialize(questTableArray);

	SendRPC(Client, Initialize, questTableArray);

#endif
}

void UQuestActorComponent::ResponseInitializeClient_Implementation(const TArray<FQuestTable>& questTableArray)
{
	if (questTableArray.IsEmpty() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("퀘스트 목록이 비어있습니다. 비정상적인 상황입니다."));
		return;
	}

	Initialize(questTableArray);

	TSubclassOf<UQuestPanel> questPanelWidgetClass = GetPlayerController()->GetQuestPanelWidgetClass();

	_questPanelWidget = CreateWidget<UQuestPanel>(GetWorld(), questPanelWidgetClass);
	_questPanelWidget->AddToViewport(QuestZOrder);
	_questPanelWidget->UpdateQuestPanel();
}

void UQuestActorComponent::Initialize(const TArray<FQuestTable>& questTableArray)
{
	ensureMsgfDebug(questTableArray.IsEmpty() == false, TEXT("퀘스트 목록이 비어있습니다. 비정상적인 상황입니다."));

	for (const FQuestTable& questTable : questTableArray)
	{
		QuestKey key(questTable._questId);

		const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(key);

		UQuestData* questData = NewObject<UQuestData>(this);

		questData->_questKey.SetKeyRawValue(key.GetKey());

		questData->_questState = static_cast<EQuestState>(questTable._questState);
		_questDataArray.Add(questData);

		if (questData->_questState == EQuestState::Clear)
		{
			_clearQuestMap.Add(key, questData);

			continue;
		}

		questData->_questCondition = MakeShared<QuestCondition>
			(
				questInfo->_clearCondition._type,
				key.GetKey(),
				questInfo->_clearCondition._conditionKeyRaw,
				questTable._conditionCount,
				questInfo->_clearCondition._clearCount
			);

		if (questData->_questState == EQuestState::Accept)
		{
			InsertQuestCondition(questData->_questCondition);
		}

		_acceptQuestMap.Add(key, questData);
	}
}

ErrNo UQuestActorComponent::ProcessUpdateQuestCondition(const EConditionType type, const int32 count, const int32 keyRaw)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == true, TEXT("서버에서 호출해 주세요."));
	ensureMsgfDebug(type != EConditionType::Count,		TEXT("비정상적인 EConditionType입니다."));
	ensureMsgfDebug(count >= 0,							TEXT("비정상적인 Count입니다."));

	QuestConditionMap& conditionMap = _questConditionMaps[GetTypeRaw(type)];
	TArray<TSharedPtr<QuestCondition>>* conditionArray = conditionMap.Find(keyRaw);
	if (conditionArray == nullptr || conditionArray->IsEmpty() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("퀘스트 확인하고 함수 호출한건데, 비어있네??"));
		return ErrNo(0);
	}

	UQuestInfoSubsystem* questInfoSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UQuestInfoSubsystem>();

	TArray<FQuestTable> questTableArray;
	for (TSharedPtr<QuestCondition> condition : *conditionArray)
	{
		const DynamicQuestInfo quesInfo = questInfoSubsystem->GetInfo(condition->GetQuestKey());
		questTableArray.Emplace(condition->GetQuestKey(), GetTypeRaw(quesInfo->_qusetType), GetTypeRaw(EQuestState::Accept), count);
	}

	ErrNo errNo = WrapperDB::UpdateQuestCondition(questTableArray, GetUserID());

	UpdateQuestCondition(type, count, keyRaw);

	SendRPC(Client, UpdateQuestCondition, GetTypeRaw(type), count, keyRaw);

	return ErrNo(0);
}


void UQuestActorComponent::ResponseUpdateQuestConditionClient_Implementation(const int32 conditionTypeRaw, const int32 count, const int32 keyRaw)
{
	if (GetEnumSize<EConditionType>() <= conditionTypeRaw)
	{
		UE_LOG(LogTemp, Warning, TEXT("EConditionType이 비정상 입니다. [%d]"), conditionTypeRaw);
		return;
	}

	if (count < 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Count가 비정상 입니다. [%d]"), count);
		return;
	}

	UpdateQuestCondition(static_cast<EConditionType>(conditionTypeRaw), count, keyRaw);
}

ErrNo UQuestActorComponent::UpdateQuestCondition(const EConditionType type, const int32 count, const int32 keyRaw)
{
	ensureMsgfDebug(type != EConditionType::Count,	TEXT("비정상적인 EConditionType입니다."));
	ensureMsgfDebug(count >= 0,						TEXT("비정상적인 Count입니다."));

	QuestConditionMap& conditionMap = _questConditionMaps[GetTypeRaw(type)];
	
	TArray<TSharedPtr<QuestCondition>>* conditionArray = conditionMap.Find(keyRaw);
	if (conditionArray == nullptr || conditionArray->IsEmpty() == true)
	{
		return errFailedApplyMemory;
	}

	// 우선 CompleteQuest함수에서 conditionArray에 있는 컨디션을 제거하기 때문에, 복사하여서 순회
	TArray<TSharedPtr<QuestCondition>> conditionArrayCache = *conditionArray;
	for (TSharedPtr<QuestCondition>& condition : conditionArrayCache)
	{
		condition->AddCurrentCount(count);
		
		if (condition->IsCompleteCondition() == true)
		{
			CompleteQuest(QuestKey(condition->GetQuestKey()));
		}

		if (GetOwner()->HasAuthority() == false)
		{
			condition->UpdateWidgetConditionCount();
		}
	}

	return ErrNo(0);
}

ErrNo UQuestActorComponent::ProcessAcceptQuest(const int32 keyRaw)
{
	if (GetOwner()->HasAuthority() == false)
	{
		ErrNo errNo = errNotServer;
		UE_LOG(LogTemp, Warning, TEXT("%s"), *errNo.GetText())
		return errNo;
	}

	QuestKey questKey(keyRaw);
	if (questKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 QuestKey입니다."));
		return errInValidRequest;
	}

	if (_acceptQuestMap.Find(questKey) != nullptr || _clearQuestMap.Find(questKey) != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 수락 또는 클리어 처리가 완료된 퀘스트 입니다. QuestKey : [%d]"), questKey.GetKey());
		return errAlreadyAcceptQuest;
	}

	const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(questKey);

	ErrNo errNo = WrapperDB::AcceptQuest(GetUserID(), questKey.GetKey(), GetTypeRaw(questInfo->_qusetType), questInfo->_clearCondition._clearCount);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *errNo.GetText());
		return errNo;
	}

	errNo = AcceptQuest(questKey);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("퀘스트 수락 서버 메모리 적용중 실패하였습니다. QuestKey : [%d]"), questKey.GetKey());
		return errNo;
	}

	SendRPC(Client, AcceptQuest, questKey.GetKey());

	return ErrNo(0);
}

void UQuestActorComponent::ResponseAcceptQuestClient_Implementation(const int32 keyRaw)
{
	QuestKey questKey(keyRaw);
	if (questKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 QuestKey입니다."));
		return;
	}

	if (_acceptQuestMap.Find(questKey) != nullptr || _clearQuestMap.Find(questKey) != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 수락 또는 클리어 처리가 완료된 퀘스트 입니다. QuestKey : [%d]"), questKey.GetKey());
		return;
	}

	ErrNo errNo = AcceptQuest(questKey);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("퀘스트 수락 클라이언트 메모리 적용 실패"));
		return;
	}

	UpdateQuestPanel();
	return;
}

ErrNo UQuestActorComponent::AcceptQuest(const QuestKey key)
{
	ensureMsgfDebug(key.IsValid() == true,					TEXT("비정상적인 QuestKey 입니다."));
	ensureMsgfDebug(_acceptQuestMap.Find(key) == nullptr,	TEXT("이미 수락한 퀘스트입니다."));
	ensureMsgfDebug(_clearQuestMap.Find(key) == nullptr,	TEXT("이미 완료한 퀘스트입니다."));

	const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(key);

	UQuestData* questData = NewObject<UQuestData>(this);
	questData->_questKey.SetKeyRawValue(key.GetKey());
	questData->_questState = EQuestState::Accept;
	questData->_questCondition = MakeShared<QuestCondition>
		(
			questInfo->_clearCondition._type,
			key.GetKey(),
			questInfo->_clearCondition._conditionKeyRaw,
			0,
			questInfo->_clearCondition._clearCount
		);

	InsertQuestCondition(questData->_questCondition);
	_acceptQuestMap.Add(key, questData);
	_questDataArray.Add(questData);

	return ErrNo(0);
}

void UQuestActorComponent::ReqClearQuest(const int32 keyRaw)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == false, TEXT("클라에서 호출해야 되는 함수입니다."));

	const QuestKey key(keyRaw);
	if (key.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 QuestKey : [%d] 입니다."), key.GetKey());
		return;
	}

	UQuestData** ppQuestData = _acceptQuestMap.Find(key);
	if (ppQuestData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("수락중인 퀘스트가 아닌데, 어떻게 요청이 들어왔지? QuestKey : [%d]"), key.GetKey());
		return;
	}

	if ((*ppQuestData)->IsCompleteQuest() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("아직 조건을 만족하지 못했는데, 어떻게 요청이 들어왔지? QuestKey : [%d]"), key.GetKey());
		return;
	}

	SendRPC(Server, ClearQuest, key.GetKey());
}

void UQuestActorComponent::ResponseClearQuestServer_Implementation(const int32 keyRaw)
{
	QuestKey questKey(keyRaw);
	if (questKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 QuestKey : [%d] 입니다."), questKey.GetKey());
		return;
	}

	UQuestData** ppQuestData = _acceptQuestMap.Find(questKey);
	if (ppQuestData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버 AcceptQuestMap에 클리어 처리할 퀘스트가 존재하지를 않네... QuestKey : [%d]"), questKey.GetKey());
		return;
	}

	ErrNo errNo = WrapperDB::ClearQuest(GetUserID(), questKey.GetKey());
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(errNo.GetText());

		UE_LOG(LogTemp, Warning, TEXT("퀘스트 완료 DB 검증에 실패하였습니다."));
		return;
	}

	errNo = ClearQuest(questKey);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(errNo.GetText());
		return;
	}

	SendRPC(Client, ClearQuest, questKey.GetKey());

	errNo = ProcessQuestReward(questKey);

	return;
}

void UQuestActorComponent::ResponseClearQuestClient_Implementation(const int32 keyRaw)
{
	QuestKey questKey(keyRaw);
	if (questKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 QuestKey : [%d] 입니다."), questKey.GetKey());
		return;
	}

	ErrNo errNo = ClearQuest(questKey);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(errNo.GetText());

		UE_LOG(LogTemp, Warning, TEXT("퀘스트 완료 클라이언트 메모리 적용 중 실패하였습니다."));
		return;
	}

	UpdateQuestPanel();

	return;
}

ErrNo UQuestActorComponent::ClearQuest(const QuestKey key)
{
	ensureMsgfDebug(key.IsValid() == true, TEXT("비정상적인 QuestKey 입니다."));

	UQuestData** ppQuestData = _acceptQuestMap.Find(key);
	if (ppQuestData == nullptr || *ppQuestData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 다 검증한건데 여기서? QuestKey : [%d]"), key.GetKey());
		return errFailedApplyMemory;
	}

	const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(key);
	if (questInfo->_qusetType == EQuestType::Repeat)
	{
		// 반복 퀘스트인 경우 다시 수락 상태로
		UQuestData* questData = *ppQuestData;

		questData->_questState = EQuestState::Accept;
		questData->_questCondition->Clear();

		InsertQuestCondition(questData->_questCondition);
	}
	else
	{
		_acceptQuestMap.Remove(key);

		UQuestData* questData = *ppQuestData;

		questData->_questState = EQuestState::Clear;
		questData->_questCondition = nullptr;

		_clearQuestMap.Add(key, questData);
	}

	return ErrNo(0);
}

ErrNo UQuestActorComponent::ProcessQuestReward(const QuestKey key)
{
	if (GetOwner()->HasAuthority() == false)
	{
		ErrNo errNo = errNotServer;
		UE_LOG(LogTemp, Warning, TEXT("%s"), *errNo.GetText())
		return errNo;
	}
	
	ensureMsgfDebug(key.IsValid() == true, TEXT("비정상적인 QuestKey 입니다."));

	const DynamicQuestInfo questInfo = GetInfoSubsystemWorld(UQuestInfoSubsystem)->GetInfo(key);
	const TArray<QuestReward>& questRewardArray = questInfo->_questRewardArray;
	
	// 같은 타입이 여러개인 경우는 없으니깐 묶어서 처리는 x
	for (const QuestReward& questReward : questRewardArray)
	{
		ErrNo errNo(0);
		switch (questReward._type)
		{
		case ERewardType::AcceptQuest:
		{
			errNo = ProcessAcceptQuest(questReward._rewardKeyRaw);
			break;
		}

		case ERewardType::GiveItem:
		{
			UInventoryActorComponent* inventory = GetPlayerController()->GetComponent<UInventoryActorComponent>();
			errNo = inventory->ProcessRewardItem(questReward._rewardKeyRaw, questReward._rewardCount);
			break;
		}

		default: break;
		}

		if (errNo.IsFailed() == true)
		{
			return errNo;
		}
	}

	return ErrNo(0);
}

TArray<UQuestData*> UQuestActorComponent::GetAcceptQuestArray()
{
	TArray<UQuestData*> questDataArray;
	_acceptQuestMap.GenerateValueArray(questDataArray);

	return questDataArray;
}

bool UQuestActorComponent::HasQuestWithCondition(const EConditionType type, const int32 keyRaw)
{
	QuestConditionMap& conditionMap = _questConditionMaps[GetTypeRaw(type)];
	TArray<TSharedPtr<QuestCondition>>* conditionArray = conditionMap.Find(keyRaw);
	if (conditionArray == nullptr || conditionArray->IsEmpty() == true)
	{
		return false;
	}

	return true;
}

void UQuestActorComponent::CompleteQuest(const QuestKey questKey)
{
	ensureMsgfDebug(questKey.IsValid() == true, TEXT("비정상적인 QuestKey입니다."));

	UQuestData** ppQuestData = _acceptQuestMap.Find(questKey);
	if (ppQuestData == nullptr || *ppQuestData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("QuestKey : [%d]가 현재 수락중인 퀘스트 목록에 존재하지 않는데 Complete요청이 들어왔습니다."), questKey.GetKey());
		return;
	}

	RemoveQuestCondition(*ppQuestData);

	(*ppQuestData)->_questState = EQuestState::Complete;
}

void UQuestActorComponent::RemoveQuestCondition(UQuestData* questData)
{
	ensureMsgfDebug(questData != nullptr, TEXT("비정상적인 QuestData입니다."));

	TSharedPtr<QuestCondition>& condition = questData->_questCondition;
	TArray<TSharedPtr<QuestCondition>>* conditionArray = _questConditionMaps[GetTypeRaw(condition->GetConditoinType())].Find(condition->GetConditionKey());
	
	int32 removeCount = conditionArray->Remove(condition);
	if (removeCount != 1)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 숫자입니다. 제거하는 개수가 1이 아닐수가 있나요? RemoveCount : [%d]"), removeCount);
		return;
	}
}

void UQuestActorComponent::InsertQuestCondition(const TSharedPtr<QuestCondition>& questCondition)
{
	QuestConditionMap& conditionMap = _questConditionMaps[GetTypeRaw(questCondition->GetConditoinType())];

	if (conditionMap.Contains(questCondition->GetConditionKey()) == false)
	{
		conditionMap.Add(questCondition->GetConditionKey(), TArray<TSharedPtr<QuestCondition>>());
	}

	conditionMap[questCondition->GetConditionKey()].Add(questCondition);
}

void UQuestActorComponent::UpdateQuestPanel()
{
	if (_questPanelWidget != nullptr)
	{
		_questPanelWidget->UpdateQuestPanel();
	}
}
