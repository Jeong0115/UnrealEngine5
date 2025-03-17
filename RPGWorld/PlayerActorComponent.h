// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerActorComponent.generated.h"

class ABasePlayerController;

#define SendRPC(Net, FuncName, ...) Response##FuncName##Net(__VA_ARGS__)

UCLASS( ClassGroup=(Custom), Blueprintable )
class RPGWORLD_API UPlayerActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerActorComponent();

	virtual void LoadDataFromDB() {}

protected:
	virtual void BeginPlay() override;

public:
	void SetPlayerController(ABasePlayerController* controller);

	ABasePlayerController* GetPlayerController() const { return _playerController; }
	int32 GetUserID() const;

private:
	ABasePlayerController* _playerController;
	
};
