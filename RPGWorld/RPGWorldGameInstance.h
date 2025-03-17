// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Global.h"
#include "RPGWorldGameInstance.generated.h"

class APlayerController;

UCLASS()
class RPGWORLD_API URPGWorldGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	virtual void Init() override;


};
