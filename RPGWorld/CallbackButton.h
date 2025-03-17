// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CallbackButton.generated.h"

class UButton;
class UImage;
class UTextBlock;

UCLASS()
class RPGWORLD_API UCallbackButton : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void			NativeConstruct() override;

	void					BindButtonCallback(TFunction<void()> func); 
	void					SetText(const FText& text);
	void					SetImage(UTexture2D* texture);

private:
	UFUNCTION()
	void					OnButtonClicked();

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton*				_button;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage*					_image;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock*				_text;

	TFunction<void()>		_callbackFunc;
};
