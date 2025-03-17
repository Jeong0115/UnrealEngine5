// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RespawnWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class RPGWORLD_API URespawnWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void    NativeConstruct() override;
	virtual void	SetVisibility(ESlateVisibility inVisibility) override;

private:
	UFUNCTION()
	void			RespawnField();

	UFUNCTION()
	void			RespawnVillage();

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton*		_respawnFieldButton;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton*		_respawnVillageButton;
	
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock*		_scrollCountText;
};
