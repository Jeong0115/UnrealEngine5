// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Global.h"
#include "StoreWidget.generated.h"

class	UButton;
class	UImage;
class	UTextBlock;
class	UVerticalBox;

struct	FItemPrice;
typedef FItemPrice ItemPrice;

UCLASS()
class RPGWORLD_API UStoreSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void	NativeConstruct() override;
	void			SetStoreData(const ItemPrice& data, const int32 index, const int32 storeKeyRaw);

private:
	UFUNCTION()
	void			Purchase();

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton*		_purchaseButton;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UImage*			_itemImage;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock*		_itemNameText;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock*		_itemDescText;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UTextBlock*		_itemPriceText;

	int32			_storeKeyRaw;
	int32			_itemPrice;
	int32			_itemKeyRaw;
	int32			_slotIndex;
};

class AStoreNPCCharacter;

UCLASS()
class RPGWORLD_API UStorePanel : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void			NativeConstruct() override;

	void					Init(const StoreKey key);

	UFUNCTION()
	void					Close();

private:
	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UVerticalBox*			_verticalBox;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton*				_closeButton;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStoreSlot> _slotClass;

	StoreKey				_key;
};
