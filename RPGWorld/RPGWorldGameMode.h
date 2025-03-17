// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Global.h"
#include "RPGWorldGameMode.generated.h"

struct	FDropItem; 
typedef FDropItem DropItem;

struct	FTimerHandle;

class	ABasePlayerController;
class	ABaseBoss;

UCLASS(minimalapi)
class ARPGWorldGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARPGWorldGameMode();

	virtual void	BeginPlay() override;
	virtual void	PostLogin(APlayerController* newPlayer) override;

	void			RewardPlayer(ABasePlayerController* controller, const MonsterKey monsterKey);

	bool			ResponseCreateAccount(ABasePlayerController* controller, const FString& id, const FString& password);
	bool			ResponseLoginAccount(ABasePlayerController* controller, const FString& id, const FString& password);

	void			DeathBossMonster(ABaseBoss* boss, AController* lastDamageInstigator);

	void			CheatSpawnBoss();

private:
	const DropItem* GetRandomDropItem(const TArray<DropItem>& dropItemArray);

	UFUNCTION()
	void			SpawnBoss();

	float			GetSecondsNextHour();

private:
	TArray<FVector> _bossSpawnPosition;
	FTimerHandle	_timerHandle;
};