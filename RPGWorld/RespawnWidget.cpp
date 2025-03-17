#include "RespawnWidget.h"

#include "Components\Button.h"
#include "Components\TextBlock.h"

#include "BasePlayerController.h"
#include "InventoryActorComponent.h"

void URespawnWidget::NativeConstruct()
{
	Super::NativeConstruct();

	_respawnFieldButton->OnClicked.AddDynamic(this,	&URespawnWidget::RespawnField);
	_respawnVillageButton->OnClicked.AddDynamic(this, &URespawnWidget::RespawnVillage);
}

void URespawnWidget::SetVisibility(ESlateVisibility inVisibility)
{
	Super::SetVisibility(inVisibility);

	if (inVisibility == ESlateVisibility::Visible)
	{
		ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
		UInventoryActorComponent* invnetory = controller->GetComponent<UInventoryActorComponent>();

		const int32 scrollCount = invnetory->GetItemCount(RevivalScrollItemKey);

		if (scrollCount > 0)
		{
			_respawnFieldButton->SetIsEnabled(true);
		}
		else
		{
			_respawnFieldButton->SetIsEnabled(false);
		}

		_scrollCountText->SetText(FText::AsNumber(scrollCount));
	}
}

void URespawnWidget::RespawnField()
{
	Cast<ABasePlayerController>(GetOwningPlayer())->ReqRespawnPlayer(true);
}

void URespawnWidget::RespawnVillage()
{
	Cast<ABasePlayerController>(GetOwningPlayer())->ReqRespawnPlayer(false);
}
