// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Global.h"
#include "PlayerActorComponent.h"
#include "QuestInfoSubsystem.h"

#include "QuestActorComponent.generated.h"

DECLARE_DELEGATE_OneParam(FChangeConditionCountDelegate, const int32);

typedef struct FQuestCondition
{
public:
	FQuestCondition() = default;
	//FQuestCondition(const FQuestCondition&) = default;
	FQuestCondition(const EConditionType type, const int32 questKey, const int32 conditionKey, const int32 currentCount, const int32 clearCount)
		: _type(type)
		, _questKeyRaw(questKey)
		, _conditionKeyRaw(conditionKey)
		, _currentCount(currentCount)
		, _clearCount(clearCount)
	{}

	void							Clear() { _currentCount = 0; }
	void							AddCurrentCount(const int32 count);
	void							UpdateWidgetConditionCount() const;

	EConditionType					GetConditoinType()		const { return _type; }
	int32							GetQuestKey()			const { return _questKeyRaw; }
	int32							GetConditionKey()		const { return _conditionKeyRaw; }
	int32							GetCurrentCount()		const { return _currentCount; }
	int32							GetClearCount()			const { return _clearCount; }
	bool							IsCompleteCondition()	const { return _currentCount >= _clearCount; }

	FChangeConditionCountDelegate	_changeCountDelegate;

private:
	EConditionType					_type;

	int32							_questKeyRaw;
	int32							_conditionKeyRaw;
	int32							_currentCount;
	int32							_clearCount;
} QuestCondition;

UCLASS()
class UQuestData : public UObject
{
	GENERATED_BODY()
public:
	FString							GetQuestName() const;
	FString							GetQuestDesc() const;
	int32							GetCurrentConditionCount() const;
	int32							GetClearConditionCount() const;
	int32							GetQuestKeyRaw() const { return _questKey.GetKey(); }

	TSharedPtr<QuestCondition>&		GetQuestCondition() { return _questCondition; }
	bool							IsCompleteQuest() const { return _questState == EQuestState::Complete; }

private:
	TSharedPtr<QuestCondition>		_questCondition;
	QuestKey						_questKey;
	EQuestState						_questState;

	friend class UQuestActorComponent;
}; 

class UQuestPanel;

UCLASS(ClassGroup = (Custom), Blueprintable)
class RPGWORLD_API UQuestActorComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

	using QuestConditionMap = TMap<int32, TArray<TSharedPtr<QuestCondition>>>;
	using QuestMap			= TMap<QuestKey, UQuestData*>;

public:	
	UQuestActorComponent();

protected:
	virtual void		BeginPlay() override;

public:
	virtual void		LoadDataFromDB() override;

	UFUNCTION(Client, Reliable)
	void				ResponseInitializeClient(const TArray<FQuestTable>& questTableArray);
	void				Initialize(const TArray<FQuestTable>& questTableArray);

	ErrNo				ProcessUpdateQuestCondition(const EConditionType type, const int32 count, const int32 keyRaw);
	
	UFUNCTION(Client, Reliable)
	void				ResponseUpdateQuestConditionClient(const int32 conditionTypeRaw, const int32 count, const int32 keyRaw);
	ErrNo				UpdateQuestCondition(const EConditionType type, const int32 count, const int32 keyRaw);
	

	ErrNo				ProcessAcceptQuest(const int32 keyRaw);

	UFUNCTION(Client, Reliable)
	void				ResponseAcceptQuestClient(const int32 keyRaw);
	ErrNo				AcceptQuest(const QuestKey key);

	void				ReqClearQuest(const int32 keyRaw);

	UFUNCTION(Server, Reliable)
	void				ResponseClearQuestServer(const int32 keyRaw);

	UFUNCTION(Client, Reliable)
	void				ResponseClearQuestClient(const int32 keyRaw);
	ErrNo				ClearQuest(const QuestKey key);

	ErrNo				ProcessQuestReward(const QuestKey key);

	UFUNCTION(BlueprintCallable)
	TArray<UQuestData*> GetAcceptQuestArray();

	bool				HasQuestWithCondition(const EConditionType type, const int32 keyRaw);

private:
	void				CompleteQuest(const QuestKey questKey);
	void				RemoveQuestCondition(UQuestData* questData);
	void				InsertQuestCondition(const TSharedPtr<QuestCondition>& questCondition);
	void				UpdateQuestPanel();

private:
	UPROPERTY()
	TArray<UQuestData*> _questDataArray;	// 실제로 직접 접근 하여 사용하는 경우는 없고, accpet할때 배열에 삽입 해줘서, GC가 인식하는 용도...

	QuestConditionMap	_questConditionMaps[GetTypeRaw(EConditionType::Count)];
	QuestMap			_acceptQuestMap;
	QuestMap			_clearQuestMap;

	UQuestPanel*		_questPanelWidget;
};
