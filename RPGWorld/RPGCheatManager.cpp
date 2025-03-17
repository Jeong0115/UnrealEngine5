// Fill out your copyright notice in the Description page of Project Settings.


#include "RPGCheatManager.h"

#include "BasePlayerController.h"
#include "BaseCharacter.h"
#include "InventoryActorComponent.h"
#include "GameInfoManagementSubsystem.h"

void URPGCheatManager::ClearAllInfo()
{
	GetInfoSubsystemWorld(UGameInfoManagementSubsystem)->CheatClearInfoArray();
}

void URPGCheatManager::ClearAllInfoEveryTick()
{
	GetInfoSubsystemWorld(UGameInfoManagementSubsystem)->CheatForceClearInfoEveryTick();
}

void URPGCheatManager::GetItem(const int32 itemKey, const int32 itemCount)
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	UInventoryActorComponent* inventory = controller->GetComponent<UInventoryActorComponent>();
	inventory->CheatGetItem(itemKey, itemCount);
}

void URPGCheatManager::GetItemNotClient(const int32 itemKey, const int32 itemCount)
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	UInventoryActorComponent* inventory = controller->GetComponent<UInventoryActorComponent>();
	inventory->CheatGetItemNotClient(itemKey, itemCount);
}

void URPGCheatManager::GetItemOnlyClient(const int32 itemKey, const int32 itemCount)
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	UInventoryActorComponent* inventory = controller->GetComponent<UInventoryActorComponent>();
	inventory->CheatGetItemOnlyClient(itemKey, itemCount);
}

void URPGCheatManager::GetItemOnlyDB(const int32 itemKey, const int32 itemCount)
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	UInventoryActorComponent* inventory = controller->GetComponent<UInventoryActorComponent>();
	inventory->CheatGetItemOnlyDB(itemKey, itemCount);
}

void URPGCheatManager::ReloadInventoryFromDB()
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	UInventoryActorComponent* inventory = controller->GetComponent<UInventoryActorComponent>();
	inventory->CheatReloadInventory();
}

void URPGCheatManager::HealPlayer(const int32 amount)
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	if (controller->GetPawn() == nullptr)
	{
		return;
	}

	Cast<ABaseCharacter>(controller->GetPawn())->CheatHeal(amount);
}

void URPGCheatManager::SetPlayerMaxHP(const int32 maxHP)
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	if (controller->GetPawn() == nullptr)
	{
		return;
	}

	Cast<ABaseCharacter>(controller->GetPawn())->CheatSetMaxHP(maxHP);
}

void URPGCheatManager::SpawnBoss()
{
	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOuterAPlayerController());
	controller->CheatSpawnBoss();
}
