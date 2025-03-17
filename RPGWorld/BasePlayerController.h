// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Global.h"
#include "BasePlayerController.generated.h"

class UPlayerActorComponent;
class UNotifyBox;
class UNotifyListPanel;
class UInventory;
class UQuestPanel;
class UStorePanel;
class URespawnWidget;
class UBossWidget;

UCLASS()
class RPGWORLD_API ABasePlayerController : public APlayerController
{
	GENERATED_BODY()

	using PlayerActorComponentMap = TMap<TSubclassOf<UPlayerActorComponent>, UPlayerActorComponent*>;

public:
	ABasePlayerController(const FObjectInitializer& objectInitializer);

	virtual void					BeginPlay() override;
	virtual void					OnPossess(APawn* InPawn) override;
	virtual void					GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void					Tick(float deltaSeconds) override;

	void							SetUserID(const int32 userId);
	void							SetUserName(const FString& userName);
	void							OpenStore(const StoreKey storeKey);
	void							CloseStore();
	void							ReqRespawnPlayer(const bool bUseRespawnScroll);
	void							WarpPlayer(const FVector& warpPosition);
	void							UseAutoPotion();

	template <typename T> T*		GetComponent()				const  { return Cast<T>(_components.FindRef(T::StaticClass())); }
	TSubclassOf<APawn>				GetControllerPawnClass()	const { return _pawnClass; }
	TSubclassOf<UQuestPanel>		GetQuestPanelWidgetClass()	const { return _questPanelClass; }
	TSubclassOf<UInventory>			GetInventoryWidgetClass()	const { return _inventoryClass; }
	TSubclassOf<UNotifyBox>			GetNotifyBoxWidgetClass()	const { return _notifyBoxClass; }

	const FString&					GetUserName()				const { return _userName; }

	UFUNCTION(BlueprintCallable)
	UPlayerActorComponent*			GetComponent(TSubclassOf<UPlayerActorComponent> componentClass) { return _components.FindRef(componentClass); }

	UFUNCTION(BlueprintPure)
	int32							GetUserID() { return _userId; }

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void							ReqCreateAccount(const FString& id, const FString& password);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void							ReqLoginAccount(const FString& id, const FString& password);

	UFUNCTION(BlueprintCallable)
	void							ToggleInventory();

	UFUNCTION(BlueprintCallable, Client, Reliable)
	void							AddNotifyMessage(const FString& message, const float time = 1.0f);

	UFUNCTION(BlueprintImplementableEvent)
	void							CreatHUDWidget();

	UFUNCTION(Client, Reliable)
	void							ShowRespawnWidget();

	UFUNCTION(Client, Reliable)
	void							HideRespawnWidget();

	UFUNCTION(Server, Reliable)
	void							RespawnPlayer(const bool bUseRespawnScroll);

	UFUNCTION(Client, Reliable)
	void							CreateSpawnBossWidget(const FVector& warpPoint, const float widgetLifeTime);

	UFUNCTION(Server, Reliable)
	void							ReqWarpPlayer(const FVector& warpPoint);

	UFUNCTION(BlueprintCallable)
	int32							GetServerGameInfoMemorySize() const;

	UFUNCTION(BlueprintCallable)
	bool							IsLogined() const { return _bLogined; }

	// ==================== Cheat Func ============================
	UFUNCTION(Server, Reliable)
	void							CheatSpawnBoss();

private:
	UFUNCTION()
	void							OnRep_Logined();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UInventory>			_inventoryClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UQuestPanel>		_questPanelClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNotifyListPanel>	_notifyListPanelClass;
	UNotifyListPanel*				_notifyListPanel;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UStorePanel>		_storePanelClass;
	UStorePanel*					_storePanel;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<URespawnWidget>		_respawnWidgetClass;
	URespawnWidget*					_respawnWidget;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UNotifyBox>			_notifyBoxClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UBossWidget>		_bossWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<APawn>				_pawnClass;

	UPROPERTY(Replicated)
	int32							_userId;

	UPROPERTY(Replicated)
	int32							_serverGameInfoMemorySize;

	UPROPERTY(ReplicatedUsing = OnRep_Logined)
	bool							_bLogined;


	PlayerActorComponentMap			_components;
	FString							_userName;
};