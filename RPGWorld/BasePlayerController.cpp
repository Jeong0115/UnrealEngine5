#include "BasePlayerController.h"

#include "Async\Async.h"
#include "Net\UnrealNetwork.h"

#include "BasePlayer.h"
#include "BaseStage.h"

#include "GameOptionActorComponent.h"
#include "InventoryActorComponent.h"
#include "QuestActorComponent.h"

#include "GameInfoManagementSubsystem.h"
#include "BossWidget.h"
#include "NotifyWidget.h"
#include "StoreWidget.h"
#include "RespawnWidget.h"

#include "RPGWorldGameMode.h"
#include "RPGWorldGameInstance.h"
#include "RPGCheatManager.h"


#define CreatePlayerActorComponent(Component) \
	U##Component* Component = CreateDefaultSubobject<U##Component>(TEXT(#Component)); \
	_components.Add(U##Component::StaticClass(), Component); \
	_components[U##Component::StaticClass()]->SetPlayerController(this)


ABasePlayerController::ABasePlayerController(const FObjectInitializer& objectInitializer)
	: Super(objectInitializer)
	, _userId(-1)
	, _serverGameInfoMemorySize(0)
	, _bLogined(false)
{
	CheatClass = URPGCheatManager::StaticClass();

	CreatePlayerActorComponent(QuestActorComponent);
	CreatePlayerActorComponent(InventoryActorComponent);
	CreatePlayerActorComponent(GameOptionActorComponent);
}

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == false)
	{
		if (_notifyListPanelClass != nullptr)
		{
			_notifyListPanel = CreateWidget<UNotifyListPanel>(this, _notifyListPanelClass);
			_notifyListPanel->AddToViewport(NotifyZOrder);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("NotifyListPanelClass가 nullptr입니다. 패널이 생성되지 않습니다."));
		}

		if (_storePanelClass != nullptr)
		{
			_storePanel = CreateWidget<UStorePanel>(this, _storePanelClass);
			_storePanel->AddToViewport(StoreZOrder);
			_storePanel->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("StorePanelClass가 nullptr입니다. 패널이 생성되지 않습니다."));
		}

		if (_respawnWidgetClass != nullptr)
		{
			_respawnWidget = CreateWidget<URespawnWidget>(this, _respawnWidgetClass);
			_respawnWidget->AddToViewport(RespawnZOrder);
			_respawnWidget->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("RespawnWidgetClass가 nullptr입니다. 패널이 생성되지 않습니다."));
		}
	}
}

void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	for (const auto& playerActorComponent : _components)
	{
		(&playerActorComponent)->Value->LoadDataFromDB();
	}
}

void ABasePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerController, _userId);
	DOREPLIFETIME(ABasePlayerController, _serverGameInfoMemorySize);
	DOREPLIFETIME(ABasePlayerController, _bLogined);
}

void ABasePlayerController::Tick(float deltaSeconds)
{
	Super::Tick(deltaSeconds);

	if (HasAuthority() == true)
	{
		_serverGameInfoMemorySize = GetInfoSubsystem(UGameInfoManagementSubsystem)->GetGameInfoMemorySize();
	}
	else
	{

	}
}

void ABasePlayerController::SetUserID(const int32 userId)
{
	if (GetWorld()->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		UE_LOG(LogTemp, Warning, TEXT("서버에서만 호출 가능한 함수입니다."));
		return;
	}

	if (_userId != -1)
	{
		UE_LOG(LogTemp, Warning, TEXT("이미 UserID가 할당이되어 있습니다. Current : [%d], Arg : [%d]"), _userId, userId);
		return;
	}

	_userId = userId;
	_bLogined = true;
}

void ABasePlayerController::SetUserName(const FString& userName)
{
	_userName = userName;
}

void ABasePlayerController::OpenStore(const StoreKey storeKey)
{
	ensureMsgfDebug(storeKey.IsValid() == true, TEXT("비정상적인 StoreKey : [%d] 입니다."), storeKey.GetKey());
	ensureMsgfDebug(HasAuthority() == false,	TEXT("클라이언트에서 호출되어야 합니다."));

	_storePanel->Init(storeKey);
	_storePanel->SetVisibility(ESlateVisibility::Visible);
}

void ABasePlayerController::CloseStore()
{
	ensureMsgfDebug(HasAuthority() == false, TEXT("클라이언트에서 호출되어야 합니다."));

	_storePanel->SetVisibility(ESlateVisibility::Collapsed);

	SetViewTargetWithBlend(GetPawn(), 0.5f);
}

void ABasePlayerController::ReqRespawnPlayer(const bool bUseRespawnScroll)
{
	ensureMsgfDebug(HasAuthority() == false, TEXT("클라이언트에 호출되어야 하는 함수입니다."));

	if (bUseRespawnScroll == true)
	{
		// 즉시 부활
		UInventoryActorComponent* inventory = GetComponent<UInventoryActorComponent>();
		inventory->ReqUseItem(RevivalScrollItemKey);
	}
	else
	{
		// 마을에서 부활
		RespawnPlayer(bUseRespawnScroll);
	}
}

void ABasePlayerController::WarpPlayer(const FVector& warpPosition)
{
	ensureMsgfDebug(HasAuthority() == true, TEXT("서버에서 호출되어야 하는 함수입니다."));

	ABasePlayer* player = Cast<ABasePlayer>(GetCharacter());
	
	player->TurnOffAutoBattle();
	player->SetActorLocation(warpPosition);
}

void ABasePlayerController::UseAutoPotion()
{
	ensureMsgfDebug(HasAuthority() == true, TEXT("서버에서 호출되어야 하는 함수입니다."));

	UInventoryActorComponent* inventory = GetComponent<UInventoryActorComponent>();
	inventory->UseAutoPotion();
}

void ABasePlayerController::ReqCreateAccount_Implementation(const FString& id, const FString& password)
{
	if (id.IsEmpty() == true || password.IsEmpty() == true)
	{
		// ...
		return;
	}

	Cast<ARPGWorldGameMode>(GetWorld()->GetAuthGameMode())->ResponseCreateAccount(this, id, password);
}

void ABasePlayerController::ReqLoginAccount_Implementation(const FString& id, const FString& password)
{
	if (id.IsEmpty() == true || password.IsEmpty() == true)
	{
		// ...
		return;
	}

	Cast<ARPGWorldGameMode>(GetWorld()->GetAuthGameMode())->ResponseLoginAccount(this, id, password);
}

void ABasePlayerController::ToggleInventory()
{
	GetComponent<UInventoryActorComponent>()->ToggleInventory();
}

void ABasePlayerController::AddNotifyMessage_Implementation(const FString& message, const float time)
{
	if (message.IsEmpty() == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Notify Message로 빈 문자열이 들어왔습니다."));
		return;
	}

	_notifyListPanel->AddNotifyMessage(message, time);
}

void ABasePlayerController::ShowRespawnWidget_Implementation()
{
	if (_respawnWidget != nullptr)
	{
		_respawnWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void ABasePlayerController::HideRespawnWidget_Implementation()
{
	if (_respawnWidget != nullptr)
	{
		_respawnWidget->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABasePlayerController::RespawnPlayer_Implementation(const bool bUseRespawnScroll)
{
	HideRespawnWidget();

	ABasePlayer* player = Cast<ABasePlayer>(GetCharacter());

	if (bUseRespawnScroll == false)
	{
		ALevelScriptActor* levelScipt = GetLevel()->GetLevelScriptActor();
		ABaseStage* baseStage = Cast<ABaseStage>(levelScipt);

		player->TurnOffAutoBattle();
		player->SetActorLocation(baseStage->GetVillagePosition());
	}

	player->Respawn(1.0f);
}

void ABasePlayerController::CreateSpawnBossWidget_Implementation(const FVector& warpPoint, const float widgetLifeTime)
{
	if (_bossWidgetClass == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("BossWidget Class를 등록해주세요"));
		return;
	}

	UBossWidget* widget = CreateWidget<UBossWidget>(this, _bossWidgetClass);
	widget->AddToViewport();
	widget->Init(warpPoint, widgetLifeTime);
}

void ABasePlayerController::ReqWarpPlayer_Implementation(const FVector& warpPoint)
{
	WarpPlayer(warpPoint);
}

int32 ABasePlayerController::GetServerGameInfoMemorySize() const
{
	return _serverGameInfoMemorySize;
}

void ABasePlayerController::CheatSpawnBoss_Implementation()
{
	Cast<ARPGWorldGameMode>(GetWorld()->GetAuthGameMode())->CheatSpawnBoss();
}

void ABasePlayerController::OnRep_Logined()
{
	if (HasAuthority() == false && _bLogined == true)
	{
		CreatHUDWidget();
	}
}	
