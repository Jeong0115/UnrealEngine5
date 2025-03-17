// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InventoryWidget.generated.h"

class UButton;
class UImage;
class UScrollBox;
class UTextBlock;
class UUniformGridPanel;

UCLASS()
class RPGWORLD_API UItemDescBox : public UUserWidget
{
    GENERATED_BODY()
public:
    void            SetText(const FString& name, const FString& desc);

private:
    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _itemNameText;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _itemDescText;
};


UCLASS()
class RPGWORLD_API UInventorySlot : public UUserWidget
{
	GENERATED_BODY()
public:
    virtual void    NativeConstruct() override;

    void            Init(UInventory* inventory, int32 slotIndex);
    void            SetItem(const int32 itemKeyRaw, const int32 itemCount);
    void            Clear();

private:
    UFUNCTION()
    void            ReqUseItem();

    UFUNCTION()
    void            ShowItemDescBox();

    UFUNCTION()
    void            HideItemDescBox();

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UButton*        _button;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UImage*         _frameImage;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UImage*         _itemImage;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UTextBlock*     _itemCountText;

    UInventory*     _inventory;

    int32           _itemKeyRaw;
    int32           _slotIndex;
};

UCLASS()
class RPGWORLD_API UInventory : public UUserWidget
{
    GENERATED_BODY()
public:
    virtual void    NativeConstruct() override;

    void            UpdateInventorySlot(const int32 slotIndex, const int32 itemKeyRaw, const int32 itemCount);

    // 0 : SlotIndex, 1 : ItemkeyRaw, 2 : ItemCount
    void            UpdateInventorySlotArray(const TArray<TTuple<int32, int32, int32 >>& updateSlotArray);
    void            ClearInventorySlot(const int32 startSlotIndex = 0);

    void            SetShowItemDescBox(const int32 slotIndex, const int32 itemKeyRaw, const bool bShow);
    void            ReqUseItem(const int32 slotIndex, const int32 itemKeyRaw);

private:
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UItemDescBox>   _itemDescBoxClass;

    UPROPERTY()
    UItemDescBox*               _itemDescBox;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UInventorySlot> _inventorySlotClass;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UUniformGridPanel*          _uniformGridPanel;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UScrollBox*                 _scrollBox;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UImage*                     _frameImage;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UImage*                     _backgroundImage;

    UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
    UButton*                    _sortButton;
};