#include "PlayerActorComponent.h"

#include "BasePlayerController.h"

UPlayerActorComponent::UPlayerActorComponent()
	: _playerController(nullptr)
{
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


void UPlayerActorComponent::BeginPlay()
{
	Super::BeginPlay();
	
}

void UPlayerActorComponent::SetPlayerController(ABasePlayerController* controller)
{
	if (controller == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerActorComponent에 할당된 PlayerController가 nullptr입니다."));
	}

	_playerController = controller;
}

int32 UPlayerActorComponent::GetUserID() const
{
	if (_playerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerController가 nullptr입니다."));
		return -1;
	}

	return _playerController->GetUserID();
}
