#include "NotifyWidget.h"

#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

#include "BasePlayerController.h"

constexpr int32 NOTIFYBOX_COUNT = 5;

void UNotifyBox::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::NativeTick(myGeometry, inDeltaTime);

	if (_bSleep == true)
	{
		return;
	}
	
	_currTime -= inDeltaTime;

	if (_currTime <= 0.f)
	{
		_backgroundImage->SetOpacity(0.f);
		_notifyText->SetOpacity(0.f);

		SleepNotify();

		return;
	}

	_notifyAlpha = _currTime / _lifeTime;

	_backgroundImage->SetOpacity(_notifyAlpha);
	_notifyText->SetOpacity(_notifyAlpha);
}

void UNotifyBox::WakeNotify()
{
	_bSleep = false;
	_notifyAlpha = 1.0f;
}

void UNotifyBox::SetText(const FString& text, const float time)
{
	_lifeTime = time;
	_currTime = time;
	_notifyText->SetText(FText::FromString(text));
}

void UNotifyBox::SleepNotify()
{
	_bSleep = true;
	_currTime = 0.f;
	_lifeTime = 0.f;
	_notifyPanel->EnqueueNotifyPool(this);
}

void UNotifyListPanel::NativeConstruct()
{
	Super::NativeConstruct();

	if (_notifyBoxClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Notify Box 클래스가 nullptr입니다."));
		return;
	}

	for (int32 i = 0; i < NOTIFYBOX_COUNT; ++i)
	{
		UNotifyBox* notifyBox = CreateWidget<UNotifyBox>(this, _notifyBoxClass);
		notifyBox->SetNotifyPanel(this);
		notifyBox->SetPadding(FMargin(0.f, 0.f, 0.f, 5.f));
		_scrollBox->AddChild(notifyBox);
		_notifyBoxQue.Enqueue(notifyBox);
	}
}

void UNotifyListPanel::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::NativeTick(myGeometry, inDeltaTime);

	if (_notifyMessageQue.IsEmpty() == true || _notifyBoxQue.IsEmpty() == true)
	{
		return;
	}

	TPair<FString, float> message;
	if (_notifyMessageQue.Peek(message) == false)
	{
		return;
	}

	UNotifyBox* notifyBox;
	if (_notifyBoxQue.Peek(notifyBox) == false)
	{
		return;
	}

	// 이거 지금 에디터 코드로 데파인
	// _scrollBox->ShiftChild(0, notifyBox);

	_scrollBox->RemoveChild(notifyBox);

	notifyBox->SetText(message.Key, message.Value);
	notifyBox->WakeNotify();

	TArray<UWidget*> tempChildArray;

	tempChildArray.Add(notifyBox);
	for (int32 i = 0; i < _scrollBox->GetChildrenCount(); i++)
	{
		tempChildArray.Add(_scrollBox->GetChildAt(i));
	}

	_scrollBox->ClearChildren();
	for (int32 i = 0; i < tempChildArray.Num(); i++)
	{
		_scrollBox->AddChild(tempChildArray[i]);
	}

	_notifyMessageQue.Pop();
	_notifyBoxQue.Pop();
}

void UNotifyListPanel::AddNotifyMessage(const FString& message, const float time)
{
	_notifyMessageQue.Enqueue({ message, time });
}

void UNotifyListPanel::EnqueueNotifyPool(UNotifyBox* notifyBox)
{
	_notifyBoxQue.Enqueue(notifyBox);
}