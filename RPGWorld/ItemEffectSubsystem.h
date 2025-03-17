// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ItemEffectSubsystem.generated.h"

class ABasePlayerController;

UCLASS()
class RPGWORLD_API UItemEffectSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    virtual void	Initialize(FSubsystemCollectionBase& collection) override;

	void			ExecuteItemEffect(const FString& funcName, ABasePlayerController* player);

private:
	void			UseLowGradePotion(ABasePlayerController* player);
	void			UseMidGradePotion(ABasePlayerController* player);
	void			UseHighGradePotion(ABasePlayerController* player);
	void			UseReturnScroll(ABasePlayerController* player);
	void			UseRevivalScroll(ABasePlayerController* player);

    TMap<FString, TFunction<void(ABasePlayerController*)>> itemEffectFuncMap;
	
};
