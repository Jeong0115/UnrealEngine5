#include "InventoryWidget.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/UniformGridPanel.h"

#include "Blueprint/SlateBlueprintLibrary.h"

#include "BasePlayerController.h"
#include "InventoryActorComponent.h"
#include "ItemInfoSubsystem.h"

constexpr int32 INVEN_COL_COUNT = 4;


void UItemDescBox::SetText(const FString& name, const FString& desc)
{
    _itemNameText->SetText(FText::FromString(name));
    _itemDescText->SetText(FText::FromString(desc));
}


void UInventorySlot::NativeConstruct()
{
    Super::NativeConstruct();

    _itemKeyRaw = ItemKey::InitValue;

    _button->OnClicked.AddDynamic(this, &UInventorySlot::ReqUseItem);
    _button->OnHovered.AddDynamic(this, &UInventorySlot::ShowItemDescBox);
    _button->OnUnhovered.AddDynamic(this, &UInventorySlot::HideItemDescBox);
}

void UInventorySlot::Init(UInventory* inventory, int32 slotIndex)
{
    _inventory = inventory;
    _slotIndex = slotIndex;
}

void UInventorySlot::SetItem(const int32 itemKeyRaw, const int32 itemCount)
{
    ensureMsgfDebug(itemCount >= 0, TEXT("비정상적인 ItemCount입니다 : [%d]"), itemCount);

    const ItemKey itemKey(itemKeyRaw);
    if (itemKey.IsValid() == false)
    {
        // 아이템 없으면 기본 값, 정상 범주
        if (_itemKeyRaw != itemKeyRaw)
        {
            _itemImage->SetBrushFromTexture(nullptr);
            _itemImage->SetOpacity(0.0f);
            _itemCountText->SetText(FText::FromString(""));

            _itemKeyRaw = itemKeyRaw;
        }
        return;
    }
    
    if (_itemKeyRaw != itemKeyRaw)
    {
        const DynamicItemInfo itemInfo = GetInfoSubsystem(UItemInfoSubsystem)->GetInfo(itemKeyRaw);

        _itemImage->SetBrushFromSoftTexture(itemInfo->_itemTexture);
        _itemImage->SetOpacity(1.0f);
        _itemKeyRaw = itemKeyRaw;
    }

    _itemCountText->SetText(FText::AsNumber(itemCount));
}

void UInventorySlot::Clear()
{
    SetItem(ItemKey::InitValue, 0);
}

void UInventorySlot::ReqUseItem()
{
    if (_itemKeyRaw != ItemKey::InitValue)
    {
        _inventory->ReqUseItem(_slotIndex, _itemKeyRaw);
    }
}

void UInventorySlot::ShowItemDescBox()
{
    if (_itemKeyRaw != ItemKey::InitValue)
    {
        _inventory->SetShowItemDescBox(_slotIndex, _itemKeyRaw, true);
    }
}

void UInventorySlot::HideItemDescBox()
{
    _inventory->SetShowItemDescBox(_slotIndex, _itemKeyRaw, false);
}

void UInventory::NativeConstruct()
{
	Super::NativeConstruct();

    if (_inventorySlotClass == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("인벤토리 슬롯 클래스가 nullptr입니다."));
        return;
    }

    ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
    UInventoryActorComponent* inventoryActorComponent = controller->GetComponent<UInventoryActorComponent>();

    const TArray<UInventoryItemData*>& invenItemArray = inventoryActorComponent->GetInventoryItemArray();

    const int32 size = invenItemArray.Num();
    for (int32 i = 0; i < size; ++i)
    {
        UInventorySlot* inventorySlot = CreateWidget<UInventorySlot>(this, _inventorySlotClass);
        _uniformGridPanel->AddChildToUniformGrid(inventorySlot, i / INVEN_COL_COUNT, i % INVEN_COL_COUNT);
        inventorySlot->Init(this, i);
        inventorySlot->SetItem(invenItemArray[i]->GetItemKeyRaw(), invenItemArray[i]->GetItemCount());
    }

    _itemDescBox = CreateWidget<UItemDescBox>(this, _itemDescBoxClass);
    _itemDescBox->AddToViewport(InvnetoryZOrder + 1);
    _itemDescBox->SetVisibility(ESlateVisibility::Collapsed);

    _sortButton->OnClicked.AddDynamic(inventoryActorComponent, &UInventoryActorComponent::ReqSortInventory);
}

void UInventory::UpdateInventorySlot(const int32 slotIndex, const int32 itemKeyRaw, const int32 itemCount)
{
    ensureMsgfDebug(slotIndex >= 0, TEXT("비정상적인 Index입니다 : [%d]"), slotIndex);
    ensureMsgfDebug(itemCount >= 0, TEXT("비정상적인 Count입니다 : [%d]"), itemCount);

    UWidget* widget = _uniformGridPanel->GetChildAt(slotIndex);
    if (widget == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("패널의 슬롯이 nullptr입니다. slotIndex : [%d]"), slotIndex);
        return;
    }

    UInventorySlot* inventorySlot = Cast<UInventorySlot>(widget);
    inventorySlot->SetItem(itemKeyRaw, itemCount);
}

void UInventory::UpdateInventorySlotArray(const TArray<TTuple<int32, int32, int32>>& updateSlotArray)
{
    for (const auto& slotInfo : updateSlotArray)
    {
        ensureMsgfDebug(slotInfo.Get<0>() >= 0, TEXT("비정상적인 Index입니다 : [%d]"), slotInfo.Get<0>());
        ensureMsgfDebug(slotInfo.Get<2>() >= 0, TEXT("비정상적인 Count입니다 : [%d]"), slotInfo.Get<2>());

        UWidget* widget = _uniformGridPanel->GetChildAt(slotInfo.Get<0>());
        if (widget == nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT("패널의 슬롯이 nullptr입니다. slotIndex : [%d]"), slotInfo.Get<0>());
            continue;
        }

        UInventorySlot* inventorySlot = Cast<UInventorySlot>(widget);
        inventorySlot->SetItem(slotInfo.Get<1>(), slotInfo.Get<2>());
    }
}

void UInventory::ClearInventorySlot(const int32 startSlotIndex)
{
    ensureMsgfDebug(startSlotIndex >= 0, TEXT("비정상적인 Index입니다 : [%d]"), startSlotIndex);

    const int32 count = _uniformGridPanel->GetChildrenCount();
    for (int32 i = startSlotIndex; i < count; ++i)
    {
        UInventorySlot* inventorySlot = Cast<UInventorySlot>(_uniformGridPanel->GetChildAt(i));
        inventorySlot->Clear();
    }
}

void UInventory::SetShowItemDescBox(const int32 slotIndex, const int32 itemKeyRaw, const bool bShow)
{
    if (bShow == false)
    {
        _itemDescBox->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        _itemDescBox->SetVisibility(ESlateVisibility::HitTestInvisible);

        UWidget* widget = _uniformGridPanel->GetChildAt(slotIndex);
        UInventorySlot* inventorySlot = Cast<UInventorySlot>(widget);
        
        const FGeometry& geometry = inventorySlot->GetCachedGeometry();
        FVector2D position, temp;

        USlateBlueprintLibrary::AbsoluteToViewport(GetWorld(), geometry.GetAbsolutePosition(), position, temp);
        
        position.X += geometry.GetAbsoluteSize().X;

        _itemDescBox->SetPositionInViewport(position);

        const DynamicItemInfo itemInfo = GetInfoSubsystem(UItemInfoSubsystem)->GetInfo(itemKeyRaw);
        _itemDescBox->SetText(itemInfo->_name, itemInfo->_desc);
    }
}

void UInventory::ReqUseItem(const int32 slotIndex, const int32 itemKeyRaw)
{
    const DynamicItemInfo itemInfo = GetInfoSubsystem(UItemInfoSubsystem)->GetInfo(itemKeyRaw);
    if (itemInfo->_bCanUseInven == false)
    {
        return;
    }

    ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
    UInventoryActorComponent* inventoryActorComponent = controller->GetComponent<UInventoryActorComponent>();

    inventoryActorComponent->ReqUseItem(itemKeyRaw, slotIndex);
}
