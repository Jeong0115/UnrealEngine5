// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Global.h"
#include "GameInfoSubsystem.generated.h"

enum class EInfoLifeSpan : uint8
{
	Temporary,
	Stage,
	Manual,
	Count
};

struct FInfoStruct
{
	virtual int32 GetMemorySize() const { return 0; }
};

template <class KeyType, class InfoType>
class DynamicInfo
{
public:
	explicit DynamicInfo(TSharedPtr<InfoType> info, const KeyType key) 
		: _info(info)
		, _key(key) 
	{}

	DynamicInfo(DynamicInfo&& other) noexcept 
		: _info(other._info)
		, _key(other._key) 
	{}

	KeyType					GetKey()		const { return _key; }
	TSharedPtr<InfoType>	operator->()	const { return _info; }

private:
	TSharedPtr<InfoType>	_info;
	KeyType					_key;
};

UCLASS()
class RPGWORLD_API UGameInfoSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void	Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void	Deinitialize() override;

	virtual void	ValidateInfoData(const FString& filePath) {};

    FString			_infoFilePath;
};
