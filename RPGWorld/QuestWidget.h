// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "QuestWidget.generated.h"

class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UVerticalBox;

class UQuestData;

UCLASS(Blueprintable)
class RPGWORLD_API UQuestBox : public UUserWidget
{
	GENERATED_BODY()
	
public:
    virtual void    NativeConstruct() override;

    void            SetQuest(UQuestData* quest);
    void            SetTitleText(const FString& text);
    void            SetDescText(const FString& text);
    void            SetCurrentCountText(const int32 count);
    void            SetTargetCountText(const int32 count);

    void            CompleteQuest(bool bComplete);

private:
    UFUNCTION()
    void            ReqClearQuest();

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UButton*        _button;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UImage*         _backgroundImage;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UImage*         _titleImage;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _titleText;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _descText;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _currentCountText;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _targetCountText;

    UPROPERTY()
    UQuestData*     _quest;
};

UCLASS()
class RPGWORLD_API UQuestPanel : public UUserWidget
{
	GENERATED_BODY()
public:
    virtual void            NativeConstruct() override;

    void                    UpdateQuestPanel();

private:
    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UScrollBox*             _scrollBox;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UVerticalBox*           _verticalBox;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UQuestBox>  _questBoxClass;
    TArray<UQuestBox*>      _questBoxArray;
};