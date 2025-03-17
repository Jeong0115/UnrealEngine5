// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseMonster.h"
#include "BaseBoss.generated.h"

/**
 * 
 */
UCLASS()
class RPGWORLD_API ABaseBoss : public ABaseMonster
{
	GENERATED_BODY()
	
public:
	virtual void	Die(AController* damageInstigator) override;
	
private:
	void DestoryBoss();

};
