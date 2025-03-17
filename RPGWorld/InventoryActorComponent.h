// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Global.h"
#include "PlayerActorComponent.h"

#include "InventoryActorComponent.generated.h"


UCLASS(Blueprintable)
class UInventoryItemData : public UObject
{
	GENERATED_BODY()
public:
	UInventoryItemData()
		: _itemKey{}, _slotIndex(-1), _itemCount(0)
	{}

	UFUNCTION(BlueprintCallable)
	FString		GetItemName() const;

	UFUNCTION(BlueprintCallable)
	FString		GetItemDesc() const;

	UFUNCTION(BlueprintCallable)
	int32		GetItemCount() const;

	UFUNCTION(BlueprintCallable)
	void		UseItem();
	
	int32		GetItemKeyRaw() const { return _itemKey.GetKey(); }

private:
	void		Clear();

	ItemKey		_itemKey;
	int32		_slotIndex;
	int32		_itemCount;

	friend class UInventoryActorComponent;
};

typedef struct FItemCollection
{
	FItemCollection()
		: _itemArray{}
		, _totalItemCount(0)
	{}

	TArray<UInventoryItemData*>	_itemArray;
	int32						_totalItemCount;
} ItemCollection;

class UInventory;


UCLASS()
class RPGWORLD_API UInventoryActorComponent : public UPlayerActorComponent
{
	GENERATED_BODY()

public:
	UInventoryActorComponent();
	
protected:
	virtual void		BeginPlay() override;

public:
	virtual void		LoadDataFromDB() override;

	UFUNCTION(Client, Reliable)
	void				ResponseInitializeClient(const TArray<FInventoryTable>& invenTableArray);
	void				Initialize(const TArray<FInventoryTable>& invenTableArray);

	void				ReloadDataFromDB();

	UFUNCTION(Client, Reliable)
	void				ResponseReloadInventoryClient(const TArray<FInventoryTable>& invenTableArray);
	void				ReInitialize(const TArray<FInventoryTable>& invenTableArray);

	void				ToggleInventory();

	ErrNo				ProcessRewardItem(const ItemKey itemKey, const int32 count);

	UFUNCTION(Client, Reliable)
	void				ResponseRewardItemClient(const TArray<FInventoryTable>& invenTableArray);
	ErrNo				RewardItem(const TArray<FInventoryTable>& invenTableArray);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void				SaveInven();

	UFUNCTION(Client, Reliable)
	void				ResponseExtendInventoryClient(const int32 extendSize);
	void				ExtendInventory(const int32 extendSize);


	void				ReqPurchaseItem(const StoreKey storeKey, const ItemKey itemKey, const int32 slotIndex);

	UFUNCTION(Server, Reliable)
	void				ResponsePurchaseItemServer(const int32 storeKeyRaw, const int32 itemKeyRaw, const int32 slotIndex);

	UFUNCTION(Client, Reliable)
	void				ResponsePurchaseItemClient(const TArray<FInventoryTable>& invenTableArray);

	ErrNo				ValidatePurchaseItem(const StoreKey storeKey, const ItemKey itemKey, const int32 slotIndex);
	ErrNo				PurchaseItem(const TArray<FInventoryTable>& invenTableArray);


	void				ReqUseItem(const ItemKey itemKey, int32 slotIndex = -1);

	UFUNCTION(Server, Reliable)
	void				ResponseUseItemServer(const int32 itemKeyRaw, const int32 slotIndex);

	UFUNCTION(Client, Reliable)
	void				ResponseUseItemClient(const int32 itemKeyRaw, const int32 slotIndex);
	ErrNo				UseItem(const int32 slotIndex);

	UFUNCTION()
	void				ReqSortInventory();

	UFUNCTION(Server, Reliable)
	void				ResponseSortInventoryServer();

	UFUNCTION(Client, Reliable)
	void				ResponseSortInventoryClient(const TArray<FInventoryTable>& invenTableArray);
	void				RollbackSortInventory(const TArray<FInventoryTable>& invenTableArray);

	UFUNCTION(BlueprintCallable)
	const TArray<UInventoryItemData*>&	GetInventoryItemArray() const;
	const UInventoryItemData*			GetInventoryItemData(const int32 slotIndex) const;

	UFUNCTION(BlueprintCallable)	
	bool				HasItem(const int32 itemKeyRaw, const int32 count) const;

	UFUNCTION(BlueprintCallable)
	bool				HasGold(const int32 count) const;

	UFUNCTION(BlueprintCallable)
	void				GetAutoPotionInfo(int32& outItemKeyRaw, int32& outItemCount);
	void				UseAutoPotion();

	int32				GetItemCount(const ItemKey itemKey) const;

	// ======================================================== Cheat Functions ========================================================
	UFUNCTION(Server, Reliable)
	void				CheatGetItem(const int32 itemKeyRaw, const int32 count);
	UFUNCTION(Server, Reliable)
	void				CheatGetItemNotClient(const int32 itemKeyRaw, const int32 count);
	UFUNCTION(Server, Reliable)
	void				CheatGetItemOnlyDB(const int32 itemKeyRaw, const int32 count);
	void				CheatGetItemOnlyClient(const int32 itemKeyRaw, const int32 count);
	UFUNCTION(Server, Reliable)
	void				CheatReloadInventory();
	// =================================================================================================================================

private:
	bool				FindSlotForGetItem(TArray<FInventoryTable>& outResult, const ItemKey itemKey, const int32 count);
	bool				FindNewSlotForGetItem(TArray<FInventoryTable>& outResult, const ItemKey itemKey, const int32 itemMaxStack, const int32 count);
	bool				FindSlotForRemoveItem(TArray<FInventoryTable>& outResult, const ItemKey itemKey, const int32 count);

	ErrNo				LoadDataAndRewardItemRetry(const ItemKey itemKey, const int32 count);

	void				RemoveItem(UInventoryItemData* itemData, const int32 count);
	void				GetItem(UInventoryItemData* itemData, const int32 count, const int32 itemKeyRaw);

	void				UpdateQuestCondition(const int32 itemKeyRaw, const int32 count, const bool bUseItem);

	void				ClearInventory();

private:
	UPROPERTY()
	TArray<UInventoryItemData*>		_inventoryItemArray;

	TMap<ItemKey, ItemCollection>	_inventoryItemMap;

	UInventory*						_inventoryWidget;
};
