// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MinimapSubsystem.generated.h"


enum class ECharacterType : uint8
{
	OtherPlayer,
	Monster
};

constexpr int32 CharacterTypeCount = 2;

class ABaseCharacter;

UCLASS()
class RPGWORLD_API UMinimapSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	void RegisterCharacterOnMinimap(const ECharacterType type, ABaseCharacter* character);
	void RemoveCharacterOnMinimap(const ECharacterType type, ABaseCharacter* character);
	
	const TArray<ABaseCharacter*>& GetDrawCharacterArray(const ECharacterType type) const;

private:
	TArray<ABaseCharacter*> _drawCharacterArray[CharacterTypeCount];
};
