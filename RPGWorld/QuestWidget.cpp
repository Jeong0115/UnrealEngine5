// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

#include "BasePlayerController.h"
#include "QuestActorComponent.h"

constexpr int32 QUESTBOX_COUNT = 5;

void UQuestBox::NativeConstruct()
{
	Super::NativeConstruct();

	_button->OnClicked.AddDynamic(this, &UQuestBox::ReqClearQuest);
}

void UQuestBox::SetQuest(UQuestData* quest)
{
	if (quest == nullptr)
	{
		if (_quest == nullptr)
		{
			_quest = nullptr;
			return;
		}

		TSharedPtr<QuestCondition>& questCondition = _quest->GetQuestCondition();
		if (questCondition.IsValid() == true && questCondition->_changeCountDelegate.IsBound() == true)
		{
			questCondition->_changeCountDelegate.Unbind();
		}

		_quest = nullptr;
		return;
	}

	_quest = quest;

	SetTitleText(_quest->GetQuestName());
	SetDescText(_quest->GetQuestDesc());
	SetCurrentCountText(_quest->GetCurrentConditionCount());
	SetTargetCountText(_quest->GetClearConditionCount());

	CompleteQuest(_quest->IsCompleteQuest());

	_quest->GetQuestCondition()->_changeCountDelegate.BindUObject(this, &UQuestBox::SetCurrentCountText);
}

void UQuestBox::SetTitleText(const FString& text)
{
	_titleText->SetText(FText::FromString(text));
}

void UQuestBox::SetDescText(const FString& text)
{
	_descText->SetText(FText::FromString(text));
}

void UQuestBox::SetCurrentCountText(const int32 count)
{
	_currentCountText->SetText(FText::AsNumber(count));

	// 조금 애매한듯...
	CompleteQuest(_quest->IsCompleteQuest());
}

void UQuestBox::SetTargetCountText(const int32 count)
{
	_targetCountText->SetText(FText::AsNumber(count));
}

void UQuestBox::CompleteQuest(bool bComplete)
{
	if (bComplete == true)
	{
		_backgroundImage->SetColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.0f, 0.3f));
	}
	else
	{
		_backgroundImage->SetColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.3f));
	}
}

void UQuestBox::ReqClearQuest()
{
	if (_quest == nullptr || _quest->IsCompleteQuest() == false)
	{
		return;
	}

	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
	UQuestActorComponent* questComponent = controller->GetComponent<UQuestActorComponent>();

	questComponent->ReqClearQuest(_quest->GetQuestKeyRaw());
}

void UQuestPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (_questBoxClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Quest Box 클래스가 nullptr입니다."));
		return;
	}

	for (int32 i = 0; i < QUESTBOX_COUNT; ++i)
	{
		UQuestBox* questBox = CreateWidget<UQuestBox>(this, _questBoxClass);
		//notifyBox->SetNotifyPanel(this);
		//notifyBox->SetPadding(FMargin(0.f, 0.f, 0.f, 5.f));
		_verticalBox->AddChild(questBox);
		_questBoxArray.Add(questBox);
	}
}

void UQuestPanel::UpdateQuestPanel()
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
	UQuestActorComponent* questComponent = controller->GetComponent<UQuestActorComponent>();

	TArray<UQuestData*> qeustArray = questComponent->GetAcceptQuestArray();

	const int32 panelSize = _questBoxArray.Num();
	const int32 size = FMath::Clamp(qeustArray.Num(), 0, panelSize);

	for (int32 i = 0; i < size; ++i)
	{
		UQuestData* questData = qeustArray[i];
		UQuestBox* questBox = _questBoxArray[i];

		_questBoxArray[i]->SetQuest(questData);
		_questBoxArray[i]->SetVisibility(ESlateVisibility::Visible);
	}

	for (int32 i = size; i < panelSize; ++i)
	{
		_questBoxArray[i]->SetQuest(nullptr);
		_questBoxArray[i]->SetVisibility(ESlateVisibility::Collapsed);
	}
}
