#include "ItemEffectSubsystem.h"

#include "BasePlayerController.h"
#include "BasePlayer.h"
#include "BaseStage.h"

void UItemEffectSubsystem::Initialize(FSubsystemCollectionBase& collection)
{
	Super::Initialize(collection);

	if (GetWorld() == nullptr|| GetWorld()->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		return;
	}

	itemEffectFuncMap.Emplace("UseLowGradePotion",	[this](ABasePlayerController* player) { UseLowGradePotion(player); });
	itemEffectFuncMap.Emplace("UseMidGradePotion",	[this](ABasePlayerController* player) { UseMidGradePotion(player); });
	itemEffectFuncMap.Emplace("UseHighGradePotion", [this](ABasePlayerController* player) { UseHighGradePotion(player); });
	itemEffectFuncMap.Emplace("UseReturnScroll",	[this](ABasePlayerController* player) { UseReturnScroll(player); });
	itemEffectFuncMap.Emplace("UseRevivalScroll",	[this](ABasePlayerController* player) { UseRevivalScroll(player); });
}

void UItemEffectSubsystem::ExecuteItemEffect(const FString& funcName, ABasePlayerController* player)
{
	if (GetWorld() == nullptr || GetWorld()->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		return;
	}

	if (itemEffectFuncMap.Contains(funcName))
	{
		itemEffectFuncMap[funcName](player);
	}
}

void UItemEffectSubsystem::UseLowGradePotion(ABasePlayerController* player)
{
	ABaseCharacter* character = Cast<ABaseCharacter>(player->GetCharacter());
	if (character == nullptr)
	{
		return;
	}

	character->Heal(50);
}

void UItemEffectSubsystem::UseMidGradePotion(ABasePlayerController* player)
{
	ABaseCharacter* character = Cast<ABaseCharacter>(player->GetCharacter());
	if (character == nullptr)
	{
		return;
	}

	character->Heal(150);
}

void UItemEffectSubsystem::UseHighGradePotion(ABasePlayerController* player)
{
	ABaseCharacter* character = Cast<ABaseCharacter>(player->GetCharacter());
	if (character == nullptr)
	{
		return;
	}

	character->Heal(300);
}

void UItemEffectSubsystem::UseReturnScroll(ABasePlayerController* player)
{
	ALevelScriptActor* levelScript = player->GetLevel()->GetLevelScriptActor();
	ABaseStage* stage = Cast<ABaseStage>(levelScript);
	if (stage == nullptr)
	{
		return;
	}
	
	player->WarpPlayer(stage->GetVillagePosition());

}

void UItemEffectSubsystem::UseRevivalScroll(ABasePlayerController* player)
{
	player->RespawnPlayer(true);
}
