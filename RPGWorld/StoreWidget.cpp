#include "StoreWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"

#include "BasePlayerController.h"
#include "StoreInfoSubsystem.h"
#include "StoreNPCCharacter.h"
#include "ItemInfoSubsystem.h"

#include "InventoryActorComponent.h"

void UStoreSlot::NativeConstruct()
{
	Super::NativeConstruct();

	_purchaseButton->OnClicked.AddDynamic(this, &UStoreSlot::Purchase);
}

void UStoreSlot::SetStoreData(const ItemPrice& data, const int32 index, const int32 storeKeyRaw)
{
	ensureMsgfDebug(data.IsValid() == true, TEXT("Store slot에 등록될 아이템 정보가 비정상입니다."));
	ensureMsgfDebug(index >= 0,				TEXT("비정상적인 SlotIndex입니다. Index : [%d]"), index);

	_storeKeyRaw	= storeKeyRaw;
	_itemPrice		= data._price;
	_itemKeyRaw		= data._itemKey.GetKey();
	_slotIndex		= index;

	const DynamicItemInfo itemInfo = GetInfoSubsystem(UItemInfoSubsystem)->GetInfo(data._itemKey);

	_itemNameText->SetText(FText::FromString(itemInfo->_name));
	_itemDescText->SetText(FText::FromString(itemInfo->_desc));
	_itemPriceText->SetText(FText::AsNumber(_itemPrice));
	_itemImage->SetBrushFromSoftTexture(itemInfo->_itemTexture);
}

void UStoreSlot::Purchase()
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
	UInventoryActorComponent* inventory = controller->GetComponent<UInventoryActorComponent>();
	inventory->ReqPurchaseItem(_storeKeyRaw, _itemKeyRaw, _slotIndex);
}

void UStorePanel::NativeConstruct()
{
	Super::NativeConstruct();

	_closeButton->OnClicked.AddDynamic(this, &UStorePanel::Close);
}

void UStorePanel::Init(const StoreKey key)
{
	_key.SetKeyRawValue(key.GetKey());

	if (_key.IsValid() == false)
	{
		return;
	}

	if (_slotClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("StoreSlot 클래스가 nullptr입니다."));
		return;
	}

	const DynamicStoreInfo storeInfo = GetInfoSubsystem(UStoreInfoSubsystem)->GetInfo(_key);

	const TArray<ItemPrice>& itemPriceArray = storeInfo->_itemPriceArray;

	const int32 currentCount = _verticalBox->GetChildrenCount();
	const int32 desiredCount = itemPriceArray.Num();

	if (currentCount < desiredCount)
	{
		for (int32 i = currentCount; i < desiredCount; ++i)
		{
			UStoreSlot* storeSlot = CreateWidget<UStoreSlot>(this, _slotClass);
			_verticalBox->AddChild(storeSlot);
		}
	}
	else if (currentCount > desiredCount)
	{
		for (int32 i = currentCount - 1; i >= desiredCount; --i)
		{
			_verticalBox->RemoveChildAt(i);
		}
	}

	for (int32 i = 0; i < desiredCount; ++i)
	{
		UWidget* widget = _verticalBox->GetChildAt(i);

		UStoreSlot* storeSlot = Cast<UStoreSlot>(widget);
		if (storeSlot == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("StoreSlot으로 캐스팅 실패 하였습니다."));
			return;
		}

		storeSlot->SetStoreData(itemPriceArray[i], i, _key.GetKey());
	}
}

void UStorePanel::Close()
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
	controller->CloseStore();
}
