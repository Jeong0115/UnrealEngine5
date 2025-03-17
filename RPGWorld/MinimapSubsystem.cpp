#include "MinimapSubsystem.h"

#include "BaseCharacter.h"

void UMinimapSubsystem::RegisterCharacterOnMinimap(const ECharacterType type, ABaseCharacter* character)
{
	_drawCharacterArray[static_cast<int32>(type)].Add(character);
}

void UMinimapSubsystem::RemoveCharacterOnMinimap(const ECharacterType type, ABaseCharacter* character)
{
	_drawCharacterArray[static_cast<int32>(type)].Remove(character);
}

const TArray<ABaseCharacter*>& UMinimapSubsystem::GetDrawCharacterArray(const ECharacterType type) const
{
	return _drawCharacterArray[static_cast<int32>(type)];
}