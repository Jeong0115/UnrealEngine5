// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Global.h"
#include "StoreNPCCharacter.generated.h"

class UWidgetComponent;
class UCallbackButton;
class ACameraActor;

UCLASS()
class RPGWORLD_API AStoreNPCCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AStoreNPCCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OpenStore();

	UFUNCTION(BlueprintNativeEvent)
	void PlayOpenStoreAnimation();

private:
	UPROPERTY(EditDefaultsOnly)
	UWidgetComponent*				_widgetComponent;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UCallbackButton>	_buttonWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	FTransform						_cameraTransform;

	UPROPERTY(EditDefaultsOnly)
	FText							_widgetText;

	UPROPERTY(EditDefaultsOnly)
	UTexture2D*						_widgetTexture;

	UPROPERTY(EditDefaultsOnly)
	int32							_storeKeyRaw;

	ACameraActor*					_storeCamera;
};
