// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "Global.h"
#include "RPGCheatManager.generated.h"

UCLASS()
class RPGWORLD_API URPGCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void ClearAllInfo();

	UFUNCTION(Exec)
	void ClearAllInfoEveryTick();

	UFUNCTION(Exec)
	void GetItem(const int32 itemKey, const int32 itemCount);

	UFUNCTION(Exec)
	void GetItemNotClient(const int32 itemKey, const int32 itemCount);

	UFUNCTION(Exec)
	void GetItemOnlyClient(const int32 itemKey, const int32 itemCount);

	UFUNCTION(Exec)
	void GetItemOnlyDB(const int32 itemKey, const int32 itemCount);

	UFUNCTION(Exec)
	void ReloadInventoryFromDB();

	UFUNCTION(Exec)
	void HealPlayer(const int32 amount);

	UFUNCTION(Exec)
	void SetPlayerMaxHP(const int32 maxHP);

	UFUNCTION(Exec)
	void SpawnBoss();
};
