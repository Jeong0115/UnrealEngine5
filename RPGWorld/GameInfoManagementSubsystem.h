// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Global.h"
#include "GameInfoSubsystem.h"
#include "GameInfoManagementSubsystem.generated.h"

struct FTimerHandle;

UCLASS()
class RPGWORLD_API UGameInfoManagementSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	using DynamicInfoArray = TArray<TSharedPtr<FInfoStruct>>;

public:
	virtual void		Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void		Deinitialize() override;

	void				RegisterDynamicInfo(TSharedPtr<FInfoStruct> info, const EInfoLifeSpan lifeSpan);

	UFUNCTION(BlueprintCallable)
	int32				GetGameInfoMemorySize() const { return _gameInfoMemorySize; }

	void				CheatClearInfoArray();
	void				CheatForceClearInfoEveryTick();

private:
	void				RemoveTemporaryInfo();

private:
	UPROPERTY()
	int32				_gameInfoMemorySize;

	DynamicInfoArray	_dynamicInfoArray[static_cast<int32>(EInfoLifeSpan::Count)];

	FTimerHandle		_timerHandle;
	bool				_bClearInfoEveryTick;
};