// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Global.h"
#include "PlayerActorComponent.h"
#include "GameOptionActorComponent.generated.h"


UCLASS()
class RPGWORLD_API UGameOptionActorComponent : public UPlayerActorComponent
{
	GENERATED_BODY()
public:
	UGameOptionActorComponent();

protected:
	virtual void		BeginPlay() override;

public:
	virtual void		GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void		LoadDataFromDB() override;
	
	UFUNCTION(Client, Reliable)
	void				ReqExtendInventoryClient();

	UFUNCTION(Server, Reliable)
	void				ResponseExtendInventoryServer();

	int32				GetCurrentInventorySize() const { return _currentInventorySize; }
	int32				GetMaxInventorySize() const { return _maxInventorySize; }

private:
	UPROPERTY(Replicated)
	int32	_currentInventorySize;

	UPROPERTY(Replicated)
	int32	_maxInventorySize;
};
