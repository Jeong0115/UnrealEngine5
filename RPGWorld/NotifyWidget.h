// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NotifyWidget.generated.h"

class UImage;
class UScrollBox;
class UTextBlock;
class UVerticalBox;

UCLASS()
class RPGWORLD_API UNotifyBox : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void		NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

	void				WakeNotify();
	void				SetText(const FString& text, const float time);
	void				SetNotifyPanel(UNotifyListPanel* notifyPanel) { _notifyPanel = notifyPanel; }

private:
	void				SleepNotify();

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage*				_backgroundImage;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock*			_notifyText;

	UNotifyListPanel*	_notifyPanel;
	float				_notifyAlpha;
	float				_currTime;
	float				_lifeTime;
	bool				_bSleep;
};


UCLASS()
class RPGWORLD_API UNotifyListPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void					NativeConstruct() override;
	virtual void					NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

	void							AddNotifyMessage(const FString& message, const float time = 1.0f);
	void							EnqueueNotifyPool(UNotifyBox* notifyBox);

private:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UScrollBox*						_scrollBox;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNotifyBox>			_notifyBoxClass;

	TDoubleLinkedList<UNotifyBox*>	_updateNotifyBoxList;

	TQueue<UNotifyBox*>				_notifyBoxQue;
	TQueue<TPair<FString, float>>	_notifyMessageQue;
};
