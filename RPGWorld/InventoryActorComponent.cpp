#include "InventoryActorComponent.h"

#include "Algo\Sort.h"

#include "DB\Public\WrapperDBInventory.h"

#include "BasePlayerController.h"
#include "GameOptionActorComponent.h"

#include "ItemInfoSubsystem.h"
#include "StoreInfoSubsystem.h"
#include "ItemEffectSubsystem.h"

#include "QuestActorComponent.h"

#include "InventoryWidget.h"

FString UInventoryItemData::GetItemName() const
{
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(_itemKey);
		return itemInfo->_name;
	}

	UE_LOG(LogTemp, Warning, TEXT("ItemName을 얻어오는 과정에서 실패하였습니다. 빈 문자열을 반환합니다."));
	return "";
}

FString UInventoryItemData::GetItemDesc() const
{
	if (GetWorld() && GetWorld()->GetGameInstance())
	{
		const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(_itemKey);
		return itemInfo->_desc;
	}

	UE_LOG(LogTemp, Warning, TEXT("ItemDesc을 얻어오는 과정에서 실패하였습니다. 빈 문자열을 반환합니다."));
	return "";
}

int32 UInventoryItemData::GetItemCount() const
{
	return _itemCount;
}

void UInventoryItemData::UseItem()
{
}

void UInventoryItemData::Clear()
{
	_itemKey.Clear();
	_itemCount = 0;
}

UInventoryActorComponent::UInventoryActorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryActorComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UInventoryActorComponent::LoadDataFromDB()
{
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	TArray<FInventoryTable> inventoryTableArray;
	ErrNo errNo = WrapperDB::LoadInventory(inventoryTableArray, GetUserID());
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("DB에서 인벤토리 데이터를 얻어오는데 실패하였습니다. : [%s]"), *errNo.GetText());
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 얻어오는데 실패하였습니다."));
		return;
	}

	Algo::Sort(inventoryTableArray, [](const FInventoryTable& first, const FInventoryTable& second) { return first._slotIndex < second._slotIndex; });

	Initialize(inventoryTableArray);
	SendRPC(Client, Initialize, inventoryTableArray);
}

void UInventoryActorComponent::ResponseInitializeClient_Implementation(const TArray<FInventoryTable>& invenTableArray)
{
	if (invenTableArray.IsEmpty() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("인벤토리가 비어있습니다."));
		return;
	}

	Initialize(invenTableArray);

	TSubclassOf<UInventory> inventoryWidgetClass = GetPlayerController()->GetInventoryWidgetClass();
	_inventoryWidget = CreateWidget<UInventory>(GetWorld(), inventoryWidgetClass);
	_inventoryWidget->AddToViewport(InvnetoryZOrder);
	_inventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
}

void UInventoryActorComponent::Initialize(const TArray<FInventoryTable>& invenTableArray)
{
	ensureMsgfDebug(invenTableArray.IsEmpty() == false, TEXT("로드 시 초기화 할 인벤토리 데이터가 비어있을 수 있나요??"));

	const int32 size = invenTableArray.Num();
	_inventoryItemArray.Reserve(size);

	for (int32 i = 0; i < size; ++i)
	{
		const int32 index = _inventoryItemArray.Emplace(NewObject<UInventoryItemData>(this));
		_inventoryItemArray[index]->_itemCount = invenTableArray[i]._itemCount;
		_inventoryItemArray[index]->_slotIndex = invenTableArray[i]._slotIndex;
		_inventoryItemArray[index]->_itemKey.SetKeyRawValue(invenTableArray[i]._itemKey);

		ItemCollection* collection = _inventoryItemMap.Find(invenTableArray[i]._itemKey);
		if (collection == nullptr)
		{
			collection = &_inventoryItemMap.Emplace(invenTableArray[i]._itemKey, FItemCollection());
		}

		collection->_itemArray.Emplace(_inventoryItemArray[index]);
		collection->_totalItemCount += invenTableArray[i]._itemCount;
	}
}

void UInventoryActorComponent::ReloadDataFromDB()
{
	if (GetOwner()->HasAuthority() == false)
	{
		return;
	}

	TArray<FInventoryTable> inventoryTableArray;
	ErrNo errNo = WrapperDB::LoadInventory(inventoryTableArray, GetUserID());
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("DB에서 인벤토리 데이터를 얻어오는데 실패하였습니다. : [%s]"), *errNo.GetText());
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 얻어오는데 실패하였습니다."));
		return;
	}

	Algo::Sort(inventoryTableArray, [](const FInventoryTable& first, const FInventoryTable& second) { return first._slotIndex < second._slotIndex; });

	ReInitialize(inventoryTableArray);
	SendRPC(Client, ReloadInventory, inventoryTableArray);
}

void UInventoryActorComponent::ResponseReloadInventoryClient_Implementation(const TArray<FInventoryTable>& invenTableArray)
{
	if (invenTableArray.IsEmpty() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("인벤토리가 비어있습니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 얻어오는데 실패하였습니다."));
		return;
	}

	ReInitialize(invenTableArray);
}


void UInventoryActorComponent::ReInitialize(const TArray<FInventoryTable>& invenTableArray)
{
	// 일단 인벤토리 확장 기능은 없으니, 인벤토리 사이즈는 동일하다는 가정하에 작성 ...
	ensureMsgfDebug(invenTableArray.IsEmpty() == false, TEXT("로드 시 초기화 할 인벤토리 데이터가 비어있을 수 있나요??"));

	const int32 size = invenTableArray.Num();
	if (_inventoryItemArray.Num() != size)
	{
		UE_LOG(LogTemp, Warning, TEXT("DB와 메모리상 존재하는 인벤토리의 크기가 다릅니다. DB size : [%d], Mem Size : [%d]"), size, _inventoryItemArray.Num());
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 얻어오는데 실패하였습니다."));
		return;
	}

	_inventoryItemMap.Empty();

	for (int32 i = 0; i < size; ++i)
	{
		_inventoryItemArray[i]->Clear();

		_inventoryItemArray[i]->_itemCount = invenTableArray[i]._itemCount;
		_inventoryItemArray[i]->_slotIndex = invenTableArray[i]._slotIndex;
		_inventoryItemArray[i]->_itemKey.SetKeyRawValue(invenTableArray[i]._itemKey);

		ItemCollection* collection = _inventoryItemMap.Find(invenTableArray[i]._itemKey);
		if (collection == nullptr)
		{
			collection = &_inventoryItemMap.Emplace(invenTableArray[i]._itemKey, FItemCollection());
		}

		collection->_itemArray.Emplace(_inventoryItemArray[i]);
		collection->_totalItemCount += invenTableArray[i]._itemCount;
	}
}


void UInventoryActorComponent::ToggleInventory()
{
	if (_inventoryWidget == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("인벤토리가 생성되지 않았습니다."));
		return;
	}

	if (_inventoryWidget->IsVisible() == true)
	{
		_inventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		_inventoryWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

ErrNo UInventoryActorComponent::ProcessRewardItem(const ItemKey itemKey, const int32 count)
{
	if (GetOwner()->HasAuthority() == false)
	{
		ErrNo errNo = errNotServer;
		UE_LOG(LogTemp, Warning, TEXT("%s"), *errNo.GetText())
		return errNo;
	}

	ensureMsgfDebug(itemKey.IsValid() == true,	TEXT("비정상적인 itemKey 입니다 : [%d]."),			itemKey.GetKey());
	ensureMsgfDebug(count >= 0,					TEXT("지급될 아이템의 개수가 음수 입니다. : [%d]"),	count);

	TArray<FInventoryTable> cacheReward;
	if (FindSlotForGetItem(cacheReward, itemKey, count) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("인벤토리 공간이 부족합니다."));
		return errNotEnoughInventory;
	}

	ErrNo errNo = WrapperDB::RewardItemToInventory(cacheReward, GetUserID());
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("요청에 실패하였습니다. 인벤토리 데이터를 다시 로드합니다."));
		ReloadDataFromDB();
		return errInValidRequest;
	}

	{
		UQuestActorComponent* questActorComponent = GetPlayerController()->GetComponent<UQuestActorComponent>();

		bool bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::GetItem, itemKey.GetKey());
		if (bUpdateCondition == true)
		{
			questActorComponent->ProcessUpdateQuestCondition(EConditionType::GetItem, count, itemKey.GetKey());
		}
	}

	errNo = RewardItem(cacheReward);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("DB 적용은 되었으나, 메모리 적용에 실패하였습니다. 인벤토리를 다시 로드합니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 다시 로드합니다."));
		ReloadDataFromDB();
		return ErrNo(0);
	}

	const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(itemKey);
	GetPlayerController()->AddNotifyMessage(FString::Printf(TEXT("%s를 %d개 획득하였습니다."), *itemInfo->_name, count));

	SendRPC(Client, RewardItem, cacheReward);

	return ErrNo(0);
}

void UInventoryActorComponent::ResponseRewardItemClient_Implementation(const TArray<FInventoryTable>& invenTableArray)
{
	if (invenTableArray.IsEmpty() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("보상 목록이 비어있습니다."));
		return;
	}

	ErrNo errNo = RewardItem(invenTableArray);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("보상 클라이언트 메모리 적용 중 실패하였습니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 아이템 데이터가 서버와 다릅니다. 인벤토리 정렬을 통해 데이터를 유지해주세요."));
		return;
	}
}

ErrNo UInventoryActorComponent::RewardItem(const TArray<FInventoryTable>& invenTableArray)
{
	ensureMsgfDebug(invenTableArray.IsEmpty() == false, TEXT("지급될 아이템 목록이 비어있습니다."));

	for (const FInventoryTable& invenTable : invenTableArray)
	{
		UInventoryItemData* inventoryItemData = _inventoryItemArray[invenTable._slotIndex];
		if (inventoryItemData->_slotIndex != invenTable._slotIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("슬롯 인덱스가 일치하지 않습니다."));
			return errFailedApplyMemory;
		}
		
		if (inventoryItemData->_itemKey != invenTable._itemKey
			&& inventoryItemData->_itemKey.IsValid() == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("아이템 키가 일치하지 않습니다."));
			return errFailedApplyMemory;
		}

		GetItem(inventoryItemData, invenTable._itemCount, invenTable._itemKey);
		
		if (GetOwner()->HasAuthority() == true)
		{
			continue;
		}

		_inventoryWidget->UpdateInventorySlot(inventoryItemData->_slotIndex, inventoryItemData->_itemKey.GetKey(), inventoryItemData->_itemCount);
	}

	return ErrNo(0);
}

void UInventoryActorComponent::SaveInven_Implementation()
{
	TArray<FInventoryTable> inventoryTableArray;
	inventoryTableArray.Reserve(_inventoryItemArray.Num());

	for (UInventoryItemData* inventoryItemData : _inventoryItemArray)
	{
		inventoryTableArray.Emplace(inventoryItemData->_slotIndex, inventoryItemData->_itemKey.GetKey(), inventoryItemData->_itemCount);
	}

	WrapperDB::SaveInventory(inventoryTableArray, GetUserID());
}

void UInventoryActorComponent::ResponseExtendInventoryClient_Implementation(const int32 extendSize)
{
	if (extendSize <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client로 전송된 확장될 인벤토리 사이즈가 0이하 입니다. size : [%d]"), extendSize);
		return;
	}
	
	const int32 maxSize = GetPlayerController()->GetComponent<UGameOptionActorComponent>()->GetMaxInventorySize();
	if (maxSize < _inventoryItemArray.Num() + extendSize)
	{
		UE_LOG(LogTemp, Warning, TEXT("최대 인벤토리 사이즈를 초과합니다. CurrentSize : [%d]"), _inventoryItemArray.Num());
		return;
	}

	ExtendInventory(extendSize);
}

void UInventoryActorComponent::ExtendInventory(const int32 extendSize)
{
	ensureMsgfDebug(extendSize > 0, TEXT("확장될 인벤토리 사이즈가 0이하 입니다. size : [%d]"), extendSize);

	const int32 firstIndex = _inventoryItemArray.AddDefaulted(extendSize);

	for (int32 i = firstIndex; i < _inventoryItemArray.Num(); ++i)
	{
		_inventoryItemArray[i] = NewObject<UInventoryItemData>(this);
	}
}

void UInventoryActorComponent::ReqPurchaseItem(const StoreKey storeKey, const ItemKey itemKey, const int32 slotIndex)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == false, TEXT("클라에서 호출해야 되는 함수입니다."));

	ErrNo errNo = ValidatePurchaseItem(storeKey, itemKey, slotIndex);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	DynamicStoreInfo storeInfo = GetInfoSubsystemWorld(UStoreInfoSubsystem)->GetInfo(storeKey);

	const TArray<ItemPrice>& itemPriceArray = storeInfo->_itemPriceArray;
	if (HasGold(itemPriceArray[slotIndex]._price) == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("구입 재화가 부족합니다."));
		return;
	}

	SendRPC(Server, PurchaseItem, storeKey.GetKey(), itemKey.GetKey(), slotIndex);
}

void UInventoryActorComponent::ResponsePurchaseItemServer_Implementation(const int32 storeKeyRaw, const int32 itemKeyRaw, const int32 slotIndex)
{
	const StoreKey storeKey(storeKeyRaw);
	const ItemKey itemKey(itemKeyRaw);

	ErrNo errNo = ValidatePurchaseItem(storeKey, itemKey, slotIndex);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(errNo.GetText());
		return;
	}

	const DynamicStoreInfo storeInfo = GetInfoSubsystemWorld(UStoreInfoSubsystem)->GetInfo(storeKey);

	const ItemPrice& itemPrice = storeInfo->_itemPriceArray[slotIndex];
	if (HasGold(itemPrice._price) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("검증하고 올라왔는데.. 클라이언트랑 가격 정보가 일치하지 않은 듯"));
		GetPlayerController()->AddNotifyMessage(TEXT("구입 재화가 부족합니다."));
		return;
	}

	TArray<FInventoryTable> cacheItem;
	if (FindSlotForRemoveItem(cacheItem, 0, itemPrice._price) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("검증하고 올라왔는데.. 클라이언트랑 인벤토리 정보가 일치하지 않은 듯"));
		GetPlayerController()->AddNotifyMessage(TEXT("구입 재화가 부족합니다."));
		return;
	}

	if (FindSlotForGetItem(cacheItem, itemKey, 1) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("검증하고 올라왔는데.. 클라이언트랑 인벤토리 정보가 일치하지 않은 듯"));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 공간이 부족합니다."));
		return;
	}


	errNo = WrapperDB::PurchaseItemToInventory(cacheItem, GetUserID());
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("요청에 실패하였습니다. 인벤토리 데이터를 다시 로드합니다."));
		ReloadDataFromDB();
		return;
	}

	{
		UQuestActorComponent* questActorComponent = GetPlayerController()->GetComponent<UQuestActorComponent>();

		bool bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::GetItem, itemKey.GetKey());
		if (bUpdateCondition == true)
		{
			questActorComponent->ProcessUpdateQuestCondition(EConditionType::GetItem, 1, itemKey.GetKey());
		}

		bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::UseItem, 0);
		if (bUpdateCondition == true)
		{
			questActorComponent->ProcessUpdateQuestCondition(EConditionType::UseItem, itemPrice._price, 0);
		}
	}

	errNo = PurchaseItem(cacheItem);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("DB 적용은 되었으나, 메모리 적용에 실패하였습니다. 인벤토리를 다시 로드합니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 다시 로드합니다."));
		ReloadDataFromDB();
		return;
	}

	SendRPC(Client, PurchaseItem, cacheItem);
}

void UInventoryActorComponent::ResponsePurchaseItemClient_Implementation(const TArray<FInventoryTable>& invenTableArray)
{
	if (invenTableArray.IsEmpty() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버에서 전송된 데이터가 비어있습니다. 비정상적인 상황입니다."));
		GetPlayerController()->AddNotifyMessage("메모리 적용에 실패하였습니다. 재 로그인 시 정상 반영됩니다.");
		return;
	}

	ErrNo errNo = PurchaseItem(invenTableArray);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("구매 클라이언 메모리 적용 중 실패하였습니다. DB, 서버에는 적용된 상태입니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 아이템 데이터가 서버와 다릅니다. 인벤토리 정렬을 통해 데이터를 유지해주세요."));
		return;
	}

	return;
}

ErrNo UInventoryActorComponent::ValidatePurchaseItem(const StoreKey storeKey, const ItemKey itemKey, const int32 slotIndex)
{
	if (storeKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 StoreKey : [%d] 입니다."), storeKey.GetKey());
		return errInValidRequest;
	}

	if (itemKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 ItemKey : [%d] 입니다."), itemKey.GetKey());
		return errInValidRequest;
	}

	const DynamicStoreInfo storeInfo = GetInfoSubsystemWorld(UStoreInfoSubsystem)->GetInfo(storeKey);

	const TArray<ItemPrice>& itemPriceArray = storeInfo->_itemPriceArray;
	if (itemPriceArray.Num() <= slotIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 SlotIndex : [%d] 입니다."), slotIndex);
		return errInValidRequest;
	}

	if (itemPriceArray[slotIndex]._itemKey != itemKey)
	{
		UE_LOG(LogTemp, Warning, TEXT("요청한 ItemKey와 Info의 데이터가 일치하지 않습니다. Req ItemKey : [%d], SlotIndex : [%d]"), itemKey.GetKey(), slotIndex);
		return errInValidRequest;
	}

	return ErrNo(0);
}

ErrNo UInventoryActorComponent::PurchaseItem(const TArray<FInventoryTable>& invenTableArray)
{
	ensureMsgfDebug(invenTableArray.IsEmpty() == false, TEXT("아이템 목록이 비어있습니다."));

	for (const FInventoryTable& invenTable : invenTableArray)
	{
		UInventoryItemData* inventoryItemData = _inventoryItemArray[invenTable._slotIndex];
		if (inventoryItemData->_slotIndex != invenTable._slotIndex)
		{
			UE_LOG(LogTemp, Warning, TEXT("슬롯 인덱스가 일치하지 않습니다."));
			return errFailedApplyMemory;
		}

		if (inventoryItemData->_itemKey != invenTable._itemKey
			&& inventoryItemData->_itemKey.IsValid() == true)
		{
			UE_LOG(LogTemp, Warning, TEXT("아이템 키가 일치하지 않습니다."));
			return errFailedApplyMemory;
		}

		if (inventoryItemData->_itemCount + invenTable._itemCount < 0)
		{
			UE_LOG(LogTemp, Warning, TEXT("아이템 개수가 음수입니다."));
			return errFailedApplyMemory;
		}
	
		if (invenTable._itemCount > 0 )
		{
			GetItem(inventoryItemData, invenTable._itemCount, invenTable._itemKey);
		}
		else
		{
			RemoveItem(inventoryItemData, invenTable._itemCount);
		}

		if (GetOwner()->HasAuthority() == false)
		{
			_inventoryWidget->UpdateInventorySlot(inventoryItemData->_slotIndex, inventoryItemData->_itemKey.GetKey(), inventoryItemData->_itemCount);
		}
	}

	return ErrNo(0);
}

void UInventoryActorComponent::ReqUseItem(const ItemKey itemKey, int32 slotIndex)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == false, TEXT("클라에서 호출해야 되는 함수입니다."));

	if (itemKey.IsValid() == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	if (slotIndex == -1)
	{
		TArray<FInventoryTable> findResult;
		if (FindSlotForRemoveItem(findResult, itemKey, 1) == false)
		{
			GetPlayerController()->AddNotifyMessage(TEXT("아이템이 존재하지 않습니다."));
			return;
		}

		slotIndex = findResult[0]._slotIndex;
	}

	if (_inventoryItemArray.Num() <= slotIndex)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(itemKey);
	if (itemInfo->_bUseEffect == false)
	{
		return;
	}

	if (_inventoryItemArray[slotIndex]->_itemKey != itemKey)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	SendRPC(Server, UseItem, itemKey.GetKey(), slotIndex);
}

void UInventoryActorComponent::ResponseUseItemServer_Implementation(const int32 itemKeyRaw, const int32 slotIndex)
{
	const ItemKey itemKey(itemKeyRaw);
	if (itemKey.IsValid() == false || _inventoryItemArray.Num() <= slotIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("클라에서 검증하고 올라온 데이터가 서버랑 다릅니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	if (_inventoryItemArray[slotIndex]->_itemKey != itemKey)
	{
		UE_LOG(LogTemp, Warning, TEXT("클라에서 검증하고 올라온 데이터가 서버랑 다릅니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	ErrNo errNo = WrapperDB::UseItemInventory(GetUserID(), slotIndex, itemKeyRaw, 1);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("요청에 실패하였습니다. 인벤토리 데이터를 다시 로드합니다."));
		ReloadDataFromDB();
		return;
	}

	{
		UQuestActorComponent* questActorComponent = GetPlayerController()->GetComponent<UQuestActorComponent>();

		bool bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::UseItem, itemKeyRaw);
		if (bUpdateCondition == true)
		{
			questActorComponent->ProcessUpdateQuestCondition(EConditionType::UseItem, 1, itemKeyRaw);
		}
	}

	const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(itemKey);
	UItemEffectSubsystem* itemEffectSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UItemEffectSubsystem>();
	itemEffectSubsystem->ExecuteItemEffect(itemInfo->_useFunc, GetPlayerController());

	errNo = UseItem(slotIndex);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("DB 반영과 아이템 사용은 성공하였지만, 메모리에 아이템 개수를 감소시키지 못하였습니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 데이터를 다시 로드합니다."));
		return;
	}

	SendRPC(Client, UseItem, itemKey.GetKey(), slotIndex);
}

void UInventoryActorComponent::ResponseUseItemClient_Implementation(const int32 itemKeyRaw, const int32 slotIndex)
{
	const ItemKey itemKey(itemKeyRaw);
	if (itemKey.IsValid() == false || _inventoryItemArray.Num() <= slotIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버에서 비정상적인 데이터가 내려왔습니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	if (_inventoryItemArray[slotIndex]->_itemKey != itemKey)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버에서 비정상적인 데이터가 내려왔습니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("유효하지 않은 요청입니다."));
		return;
	}

	ErrNo errNo = UseItem(slotIndex);
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("DB 반영과 아이템 사용은 성공하였지만, 메모리에 아이템 개수를 감소시키지 못하였습니다."));
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 아이템 데이터가 서버와 다릅니다. 인벤토리 정렬을 통해 데이터를 유지해주세요."));
		return;
	}
}

ErrNo UInventoryActorComponent::UseItem(const int32 slotIndex)
{
	ensureMsgfDebug(_inventoryItemArray.Num() > slotIndex, TEXT("비정상적인 SlotIndex : [%d] 입니다."), slotIndex);

	UInventoryItemData* itemData = _inventoryItemArray[slotIndex];
	RemoveItem(itemData, -1);

	if (GetOwner()->HasAuthority() == false)
	{
		_inventoryWidget->UpdateInventorySlot(itemData->_slotIndex, itemData->_itemKey.GetKey(), itemData->_itemCount);
	}

	return ErrNo(0);
}

void UInventoryActorComponent::ReqSortInventory()
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == false, TEXT("클라에서 호출해야 되는 함수입니다."));

	SendRPC(Server, SortInventory);

	// 서버로 요청 후, 미리 메모리 적용 
	TArray<ItemKey> sortKeyArray;
	_inventoryItemMap.GenerateKeyArray(sortKeyArray);

	sortKeyArray.Sort([](const ItemKey& front, const ItemKey& back)
		{
			return front.GetKey() < back.GetKey();
		});

	if (sortKeyArray[0].IsValid() == false)
	{
		sortKeyArray.RemoveAt(0);
	}

	int32 slotIndex = 0;

	for (auto& pairCollection : _inventoryItemMap)
	{
		ItemCollection& collection = pairCollection.Value;

		for (UInventoryItemData* itemData : collection._itemArray)
		{
			itemData->Clear();
		}

		collection._itemArray.Empty();
	}

	// 0 : SlotIndex, 1 : ItemkeyRaw, 2 : ItemCount
	TArray<TTuple<int32, int32, int32>> updateWidgetInfo;
	updateWidgetInfo.Reserve(_inventoryItemArray.Num());

	for (const ItemKey& key : sortKeyArray)
	{
		const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(key);
		const int32 maxStack = itemInfo->_maxStack;

		int32 remainCount = _inventoryItemMap[key]._totalItemCount;

		while (remainCount > 0)
		{
			UInventoryItemData* itemData = _inventoryItemArray[slotIndex++];
			itemData->_itemKey.SetKeyRawValue(key.GetKey());
			itemData->_itemCount = FMath::Min(maxStack, remainCount);

			_inventoryItemMap[key]._itemArray.Emplace(itemData);

			updateWidgetInfo.Emplace(itemData->_slotIndex, itemData->_itemKey.GetKey(), itemData->_itemCount);

			remainCount -= maxStack;
		}
	}
	
	_inventoryWidget->UpdateInventorySlotArray(updateWidgetInfo);

	if (_inventoryItemArray.Num() > slotIndex)
	{
		ItemCollection* emptyCollection = &_inventoryItemMap.Emplace(ItemKey::InitValue, FItemCollection());
		emptyCollection->_itemArray.Append(_inventoryItemArray.GetData() + slotIndex, _inventoryItemArray.Num() - slotIndex);

		_inventoryWidget->ClearInventorySlot(slotIndex);
	}
}

void UInventoryActorComponent::ResponseSortInventoryServer_Implementation()
{
	TArray<ItemKey> sortKeyArray;
	_inventoryItemMap.GenerateKeyArray(sortKeyArray);

	sortKeyArray.Sort([](const ItemKey& front, const ItemKey& back)
		{
			return front.GetKey() < back.GetKey();
		});
	
	// 첫번째에 비어있는 슬롯에 할당된 ItemKey가 들어있을 수도
	if (sortKeyArray[0].IsValid() == false)
	{
		sortKeyArray.RemoveAt(0);
	}

	TArray<FInventoryTable> resultArray;
	resultArray.Reserve(_inventoryItemArray.Num());

	int32 slotIndex = 0;

	for (const ItemKey& key : sortKeyArray)
	{
		const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(key.GetKey());
		const int32 maxStack = itemInfo->_maxStack;

		int32 remainCount = _inventoryItemMap[key]._totalItemCount;
		while (remainCount > 0)
		{
			resultArray.Emplace(slotIndex++, key.GetKey(), FMath::Min(maxStack, remainCount));
			remainCount -= maxStack;
		}
	}

	ErrNo errNo = WrapperDB::SortInventory(resultArray, GetUserID());
	if (errNo.IsFailed())
	{
		UE_LOG(LogTemp, Warning, TEXT("인벤토리 정렬 DB반영 실패"));
		GetPlayerController()->AddNotifyMessage(errNo.GetText());
		return;
	}

	for (auto& pairCollection : _inventoryItemMap)
	{
		ItemCollection& collection = pairCollection.Value;

		for (UInventoryItemData* itemData : collection._itemArray)
		{
			itemData->Clear();
		}
	}
	_inventoryItemMap.Empty();

	for (const FInventoryTable& result : resultArray)
	{
		UInventoryItemData* itemData = _inventoryItemArray[result._slotIndex];
		itemData->_itemKey.SetKeyRawValue(result._itemKey);
		itemData->_itemCount = result._itemCount;

		ItemCollection* collection = _inventoryItemMap.Find(itemData->_itemKey);
		if (collection == nullptr)
		{
			collection = &_inventoryItemMap.Emplace(itemData->_itemKey, FItemCollection());
		}

		collection->_itemArray.Emplace(itemData);
		collection->_totalItemCount += itemData->_itemCount;
	}

	if (_inventoryItemArray.Num() > slotIndex)
	{
		ItemCollection* emptyCollection = &_inventoryItemMap.Emplace(ItemKey::InitValue, FItemCollection());
		emptyCollection->_itemArray.Append(_inventoryItemArray.GetData() + slotIndex, _inventoryItemArray.Num() - slotIndex);
	}

	SendRPC(Client, SortInventory, resultArray);
}

void UInventoryActorComponent::ResponseSortInventoryClient_Implementation(const TArray<FInventoryTable>& invenTableArray)
{
	for (const FInventoryTable& invenTable : invenTableArray)
	{
		if (_inventoryItemArray.IsValidIndex(invenTable._slotIndex) == false)
		{
			UE_LOG(LogTemp, Warning, TEXT("비정상적인 SlotIndex : [%d] 입니다."), invenTable._slotIndex);
			GetPlayerController()->AddNotifyMessage(TEXT("메모리 적용에 실패하였습니다.재접속 시 정상 반영됩니다."));

			return;
		}

		UInventoryItemData* itemData = _inventoryItemArray[invenTable._slotIndex];

		if (itemData->_itemKey.GetKey() != invenTable._itemKey
			|| itemData->_itemCount != invenTable._itemCount)
		{
			UE_LOG(LogTemp, Warning, TEXT("서버와 클라이언트의 인벤토리 정렬 결과가 다릅니다. 서버 데이터로 덮어 씌웁니다."));
			
			// 노티는 굳이?
			RollbackSortInventory(invenTableArray);

			return;
		}
	}
}

void UInventoryActorComponent::RollbackSortInventory(const TArray<FInventoryTable>& invenTableArray)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == false, TEXT("클라에서 호출해야 되는 함수입니다."));

	for (auto& pairCollection : _inventoryItemMap)
	{
		ItemCollection& collection = pairCollection.Value;

		for (UInventoryItemData* itemData : collection._itemArray)
		{
			itemData->Clear();
		}
	}
	_inventoryItemMap.Empty();

	// 0 : SlotIndex, 1 : ItemkeyRaw, 2 : ItemCount
	TArray<TTuple<int32, int32, int32>> updateWidgetInfo;
	updateWidgetInfo.Reserve(_inventoryItemArray.Num());

	for (const FInventoryTable& result : invenTableArray)
	{
		UInventoryItemData* itemData = _inventoryItemArray[result._slotIndex];
		itemData->_itemKey.SetKeyRawValue(result._itemKey);
		itemData->_itemCount = result._itemCount;

		ItemCollection* collection = _inventoryItemMap.Find(itemData->_itemKey);
		if (collection == nullptr)
		{
			collection = &_inventoryItemMap.Emplace(itemData->_itemKey, FItemCollection());
		}

		collection->_itemArray.Emplace(itemData);
		collection->_totalItemCount += itemData->_itemCount;

		updateWidgetInfo.Emplace(itemData->_slotIndex, itemData->_itemKey.GetKey(), itemData->_itemCount);
	}

	_inventoryWidget->UpdateInventorySlotArray(updateWidgetInfo);

	const int32 tableCount = invenTableArray.Num();
	if (_inventoryItemArray.Num() > tableCount)
	{
		ItemCollection* emptyCollection = &_inventoryItemMap.Emplace(ItemKey::InitValue, FItemCollection());
		emptyCollection->_itemArray.Append(_inventoryItemArray.GetData() + tableCount, _inventoryItemArray.Num() - tableCount);

		_inventoryWidget->ClearInventorySlot(tableCount);
	}
}

const TArray<UInventoryItemData*>& UInventoryActorComponent::GetInventoryItemArray() const
{
	return _inventoryItemArray;
}

const UInventoryItemData* UInventoryActorComponent::GetInventoryItemData(const int32 slotIndex) const
{
	if (_inventoryItemArray.Num() <= slotIndex)
	{
		UE_LOG(LogTemp, Warning, TEXT("배열 범위를 초과한 인덱스 입니다. Index : [%d], Size : [%d]"), slotIndex, _inventoryItemArray.Num());
		return nullptr;
	}

	return _inventoryItemArray[slotIndex];
}

bool UInventoryActorComponent::HasItem(const int32 itemKeyRaw, const int32 count) const
{
	const ItemKey itemKey(itemKeyRaw);

	ensureMsgfDebug(itemKey.IsValid() == true,	TEXT("비정상적인 itemKey입니다. : [%d]"), itemKey.GetKey());
	ensureMsgfDebug(count >= 0,					TEXT("확인할 아이템 개수가 0보다 작습니다. Count : [%d]"), count);

	const ItemCollection* itemCollection = _inventoryItemMap.Find(itemKey);
	if (itemCollection == nullptr)
	{
		return false;
	}

	if (itemCollection->_totalItemCount < count)
	{
		return false;
	}

	return true;
}

bool UInventoryActorComponent::HasGold(const int32 count) const
{
	ensureMsgfDebug(count >= 0, TEXT("확인할 아이템 개수가 0보다 작습니다. Count : [%d]"), count);

	return HasItem(0, count);
}

void UInventoryActorComponent::UseAutoPotion()
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == true, TEXT("서버에서 호출해야 되는 함수입니다."));

	// 이런것도 다 데이터로 정리하면 좋겠지만..
	static TArray<int32> potionKeyArray = { 3, 2, 1 };

	for (const int32 keyRaw : potionKeyArray)
	{
		TArray<FInventoryTable> findResult;

		if (FindSlotForRemoveItem(findResult, keyRaw, 1) == true)
		{
			const int32 slotIndex = findResult[0]._slotIndex;
			ResponseUseItemServer(keyRaw, slotIndex);

			return;
		}
	}
}

void UInventoryActorComponent::GetAutoPotionInfo(int32& outItemKeyRaw, int32& outItemCount)
{
	// 이런것도 다 데이터로 정리하면 좋겠지만..
	static TArray<int32> potionKeyArray = { 3, 2, 1 };

	for (const int32 keyRaw : potionKeyArray)
	{
		int32 count = GetItemCount(keyRaw);

		if (count > 0)
		{
			outItemKeyRaw	= keyRaw;
			outItemCount	= count;

			return;
		}
	}

	outItemKeyRaw	= ItemKey::InitValue;
	outItemCount	= 0;
}

int32 UInventoryActorComponent::GetItemCount(const ItemKey itemKey) const
{
	ensureMsgfDebug(itemKey.IsValid() == true, TEXT("비정상적인 itemKey입니다. : [%d]"), itemKey.GetKey());

	const ItemCollection* itemCollection = _inventoryItemMap.Find(itemKey);
	if (itemCollection == nullptr)
	{
		return 0;
	}

	return itemCollection->_totalItemCount;
}

void UInventoryActorComponent::CheatGetItem_Implementation(const int32 itemKeyRaw, const int32 count)
{
	const ItemKey itemKey(itemKeyRaw);
	if (itemKey.IsValid() == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("비정상적인 itemKey 입니다"));
		return;
	}

	if (count <= 0)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("지급할 아이템 개수를 1이상으로 정해주세요."));
		return;
	}

	ProcessRewardItem(itemKey, count);
}

void UInventoryActorComponent::CheatGetItemNotClient_Implementation(const int32 itemKeyRaw, const int32 count)
{
	const ItemKey itemKey(itemKeyRaw);
	if (itemKey.IsValid() == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("비정상적인 itemKey 입니다"));
		return;
	}

	if (count <= 0)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("지급할 아이템 개수를 1이상으로 정해주세요."));
		return;
	}

	TArray<FInventoryTable> cacheReward;
	if (FindSlotForGetItem(cacheReward, itemKey, count) == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("치트 실패"));
		return;
	}

	ErrNo errNo = WrapperDB::RewardItemToInventory(cacheReward, GetUserID());
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("치트 실패"));
		return;
	}

	errNo = RewardItem(cacheReward);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("DB는 성공하였으나, 서버 메모리 적용 실패"));
		return;
	}
}

void UInventoryActorComponent::CheatGetItemOnlyDB_Implementation(const int32 itemKeyRaw, const int32 count)
{
	const ItemKey itemKey(itemKeyRaw);
	if (itemKey.IsValid() == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("비정상적인 itemKey 입니다"));
		return;
	}

	if (count <= 0)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("지급할 아이템 개수를 1이상으로 정해주세요."));
		return;
	}

	TArray<FInventoryTable> cacheReward;
	if (FindSlotForGetItem(cacheReward, itemKey, count) == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 공간이 부족합니다."));
		return;
	}

	ErrNo errNo = WrapperDB::RewardItemToInventory(cacheReward, GetUserID());
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("DB반영에 실패하였습니다."));
		return;
	}
}

void UInventoryActorComponent::CheatGetItemOnlyClient(const int32 itemKeyRaw, const int32 count)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == false, TEXT("클라에서 호출해야 되는 함수입니다."));

	const ItemKey itemKey(itemKeyRaw);
	if (itemKey.IsValid() == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("비정상적인 itemKey 입니다"));
		return;
	}

	if (count <= 0)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("지급할 아이템 개수를 1이상으로 정해주세요."));
		return;
	}

	TArray<FInventoryTable> cacheTable;
	if (FindSlotForGetItem(cacheTable, itemKey, count) == false)
	{
		GetPlayerController()->AddNotifyMessage(TEXT("인벤토리 공간이 부족합니다."));
		return;
	}

	ErrNo errNo = RewardItem(cacheTable);
	if (errNo.IsFailed() == true)
	{
		GetPlayerController()->AddNotifyMessage(errNo.GetText());
		return;
	}
}

void UInventoryActorComponent::CheatReloadInventory_Implementation()
{
	ReloadDataFromDB();
}

bool UInventoryActorComponent::FindSlotForGetItem(TArray<FInventoryTable>& outResult, const ItemKey itemKey, const int32 count)
{
	ensureMsgfDebug(itemKey.IsValid() == true,	TEXT("비정상적인 itemKey 입니다 : [%d]."),			itemKey.GetKey());
	ensureMsgfDebug(count >= 0,					TEXT("지급될 아이템의 개수가 음수 입니다. : [%d]"),	count);

	const DynamicItemInfo itemInfo = GetInfoSubsystemWorld(UItemInfoSubsystem)->GetInfo(itemKey);
	const int32 itemMaxStack = itemInfo->_maxStack;

	ItemCollection* collection = _inventoryItemMap.Find(itemKey);
	if (collection == nullptr)
	{
		return FindNewSlotForGetItem(outResult, itemKey, itemMaxStack, count);
	}

	int32 remainItemCount = count;
	for (UInventoryItemData* invenItemData : collection->_itemArray)
	{
		if (invenItemData->_itemCount >= itemMaxStack)
		{
			continue;
		}

		if (invenItemData->_itemCount + remainItemCount > itemMaxStack)
		{
			int32 rewardCount = itemMaxStack - invenItemData->_itemCount;
			remainItemCount -= rewardCount;

			outResult.Emplace(invenItemData->_slotIndex, itemKey.GetKey(), rewardCount);
		}
		else
		{
			outResult.Emplace(invenItemData->_slotIndex, itemKey.GetKey(), remainItemCount);
			remainItemCount = 0;
			break;
		}
	}

	if (remainItemCount != 0)
	{
		return FindNewSlotForGetItem(outResult, itemKey, itemMaxStack, remainItemCount);
	}

	return true;
}

bool UInventoryActorComponent::FindNewSlotForGetItem(TArray<FInventoryTable>& outResult, const ItemKey itemKey, const int32 itemMaxStack, const int32 count)
{
	ensureMsgfDebug(itemKey.IsValid() == true,	TEXT("비정상적인 itemKey 입니다 : [%d]."),			itemKey.GetKey());
	ensureMsgfDebug(count >= 0,					TEXT("지급될 아이템의 개수가 음수 입니다. : [%d]"),	count);
	ensureMsgfDebug(itemMaxStack > 0,			TEXT("비정상적인 아이템 스택 개수입니다. : [%d]"),		itemMaxStack);

	ItemCollection* collection = _inventoryItemMap.Find(ItemKey());
	if (collection == nullptr)
	{
		return false;
	}

	int32 remainItemCount = count;
	for (UInventoryItemData* invenItemData : collection->_itemArray)
	{
		if (invenItemData->_itemCount >= itemMaxStack)
		{
			continue;
		}

		if (invenItemData->_itemCount + remainItemCount > itemMaxStack)
		{
			int32 rewardCount = itemMaxStack - invenItemData->_itemCount;
			remainItemCount -= rewardCount;

			outResult.Emplace(invenItemData->_slotIndex, itemKey.GetKey(), rewardCount);
		}
		else
		{
			outResult.Emplace(invenItemData->_slotIndex, itemKey.GetKey(), remainItemCount);

			remainItemCount = 0;
			break;
		}
	}

	if (remainItemCount != 0)
	{
		return false;
	}

	return true;
}

bool UInventoryActorComponent::FindSlotForRemoveItem(TArray<FInventoryTable>& outResult, const ItemKey itemKey, const int32 count)
{
	ensureMsgfDebug(itemKey.IsValid() == true,	TEXT("비정상적인 itemKey 입니다 : [%d]."), itemKey.GetKey());
	ensureMsgfDebug(count >= 0,					TEXT("사용할 아이템의 개수가 음수 입니다. : [%d]"), count);

	ItemCollection* collection = _inventoryItemMap.Find(itemKey);
	if (collection == nullptr)
	{
		return false;
	}

	int32 remainItemCount = count;
	for (UInventoryItemData* invenItemData : collection->_itemArray)
	{
		outResult.Emplace(invenItemData->_slotIndex, itemKey.GetKey(), -FMath::Min(invenItemData->_itemCount, remainItemCount));

		remainItemCount -= invenItemData->_itemCount;
		if (remainItemCount <= 0)
		{
			break;
		}
	}

	if (remainItemCount > 0)
	{
		return false;
	}

	return true;
}

ErrNo UInventoryActorComponent::LoadDataAndRewardItemRetry(const ItemKey itemKey, const int32 count)
{
	return ErrNo();
}

void UInventoryActorComponent::RemoveItem(UInventoryItemData* itemData, const int32 count)
{
	// 그냥 양수로 전달해서 감소시키는게 더 직관적일듯... 우선 유지하자
	ensureMsgfDebug(count <= 0, TEXT("UseItem에 전달된 count는 음수로 전달되어야 합니다."));

	ItemCollection* collection = _inventoryItemMap.Find(itemData->_itemKey);
	collection->_totalItemCount += count;
	itemData->_itemCount += count;

	if (itemData->_itemCount > 0)
	{
		return;
	}

	collection->_itemArray.Remove(itemData);
	if (collection->_totalItemCount == 0)
	{
		_inventoryItemMap.Remove(itemData->_itemKey);
	}

	itemData->_itemKey.Clear();

	ItemCollection* newCollection = _inventoryItemMap.Find(itemData->_itemKey);
	if (newCollection == nullptr)
	{
		newCollection = &_inventoryItemMap.Emplace(itemData->_itemKey, FItemCollection());
	}

	newCollection->_itemArray.Add(itemData);
}

void UInventoryActorComponent::GetItem(UInventoryItemData* itemData, const int32 count, const int32 itemKeyRaw)
{
	ensureMsgfDebug(count >= 0, TEXT("GetItem에 전달된 count는 양수로 전달되어야 합니다."));

	if (itemData->_itemKey.IsValid() == false)
	{
		ItemCollection* oldCollection = _inventoryItemMap.Find(itemData->_itemKey);
		oldCollection->_itemArray.Remove(itemData);

		itemData->_itemKey.SetKeyRawValue(itemKeyRaw);
		itemData->_itemCount += count;

		ItemCollection* newCollection = _inventoryItemMap.Find(itemData->_itemKey);
		if (newCollection == nullptr)
		{
			newCollection = &_inventoryItemMap.Emplace(itemData->_itemKey, FItemCollection());
		}

		newCollection->_itemArray.Add(itemData);
		newCollection->_totalItemCount += count;
	}
	else
	{
		ItemCollection* collection = _inventoryItemMap.Find(itemData->_itemKey);
		collection->_totalItemCount += count;
		itemData->_itemCount += count;
	}
}

void UInventoryActorComponent::UpdateQuestCondition(const int32 itemKeyRaw, const int32 count, const bool bUseItem)
{
	ensureMsgfDebug(GetOwner()->HasAuthority() == true, TEXT("서버에서 호출해 주세요."));

	UQuestActorComponent* questActorComponent = GetPlayerController()->GetComponent<UQuestActorComponent>();

	if (bUseItem == true)
	{
		bool bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::UseItem, itemKeyRaw);
		if (bUpdateCondition == true)
		{
			questActorComponent->ProcessUpdateQuestCondition(EConditionType::UseItem, count, itemKeyRaw);
		}

	}
	else
	{
		bool bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::GetItem, itemKeyRaw);
		if (bUpdateCondition == true)
		{
			questActorComponent->ProcessUpdateQuestCondition(EConditionType::GetItem, count, itemKeyRaw);
		}

	}
}

void UInventoryActorComponent::ClearInventory()
{
	for (UInventoryItemData* itemData : _inventoryItemArray)
	{
		if (itemData != nullptr)
		{
			itemData->ConditionalBeginDestroy();
		}
	}
	_inventoryItemArray.Empty();

	for (auto& Entry : _inventoryItemMap)
	{
		ItemCollection& collection = Entry.Value;

		for (UInventoryItemData* itemData : collection._itemArray)
		{
			if (itemData != nullptr)
			{
				itemData->ConditionalBeginDestroy();
			}
		}

		collection._itemArray.Empty();
		collection._totalItemCount = 0;
	}
	_inventoryItemMap.Empty();
}
