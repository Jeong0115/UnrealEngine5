// Copyright Epic Games, Inc. All Rights Reserved.

#include "RPGWorldGameMode.h"

#include <string>

#include "Math\UnrealMathUtility.h"
#include "Misc\DateTime.h"
#include "DB\Public\WrapperDBLogin.h"
#include "TimerManager.h"

#include "BaseMonster.h"
#include "BaseBoss.h"
#include "BasePlayerController.h"
#include "RPGWorldGameInstance.h"

#include "InventoryActorComponent.h"
#include "QuestActorComponent.h"

#include "DropGroupInfoSubsystem.h"
#include "MonsterInfoSubsystem.h"

ARPGWorldGameMode::ARPGWorldGameMode()
	: _bossSpawnPosition
		{ 
			{5650.f, 5650.f, 92.f}	,
			{5650.f, -2150.f, 92.f} ,
			{-2150.f, 5650.f, 92.f} ,
			{-2150.f, -2150.f, 92.f} 
		}
{
	DefaultPawnClass = nullptr;
	bUseSeamlessTravel = true;
}

void ARPGWorldGameMode::BeginPlay()
{
	Super::BeginPlay();

	const float beginDelay = GetSecondsNextHour();

	GetWorldTimerManager().SetTimer(_timerHandle, this, &ARPGWorldGameMode::SpawnBoss, beginDelay, false);
}

void ARPGWorldGameMode::PostLogin(APlayerController* newPlayer)
{
	Super::PostLogin(newPlayer);
}

void ARPGWorldGameMode::RewardPlayer(ABasePlayerController* controller, const MonsterKey monsterKey)
{
	ensureMsgfDebug(controller != nullptr,			TEXT("Controller가 nullptr입니다."));
	ensureMsgfDebug(monsterKey.IsValid() == true,	TEXT("비정상적인 MonsterKey 입니다. key : [%d]"), monsterKey.GetKey());

	const DynamicMonsterInfo monsterInfo = GetInfoSubsystem(UMonsterInfoSubsystem)->GetInfo(monsterKey);
	if (monsterInfo->_dropGroupKey.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("몬스터의 DropGroupKey가 비정상 입니다. MonsterKey : [%d]"), monsterKey.GetKey());
		return;
	}

	UQuestActorComponent* questActorComponent = controller->GetComponent<UQuestActorComponent>();

	bool bUpdateCondition = questActorComponent->HasQuestWithCondition(EConditionType::KillMonster, monsterKey.GetKey());
	if (bUpdateCondition == true)	
	{
		questActorComponent->ProcessUpdateQuestCondition(EConditionType::KillMonster, 1, monsterKey.GetKey());
	}

	const DynamicDropGroupInfo dropGroupInfo = GetInfoSubsystem(UDropGroupInfoSubsystem)->GetInfo(monsterInfo->_dropGroupKey);
	const TArray<DropItem>& dropItemArray = dropGroupInfo->GetDropItemArray();

	const DropItem* dropItem = GetRandomDropItem(dropItemArray);
	if (dropItem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("DropItem이 선택되지 않았습니다. DropGroupKey : [%d]"), monsterInfo->_dropGroupKey.GetKey());
		return;
	}

	const ItemKey itemKey = dropItem->_itemKey;
	dropItem->_itemCount;

	controller->GetComponent<UInventoryActorComponent>()->ProcessRewardItem(itemKey, dropItem->_itemCount);
}

bool ARPGWorldGameMode::ResponseCreateAccount(ABasePlayerController* controller, const FString& id, const FString& password)
{
	if (controller->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("유효하지 않은 컨트롤러 입니다."));
		return false;
	}

	if (id.IsEmpty() || password.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 아이디 : [%s] 혹은, 비밀번호 : [%s] 입니다."), *id, *password);
		return false;
	}

	const std::string strID = TCHAR_TO_UTF8(*id);
	const std::string strPassword = TCHAR_TO_UTF8(*password);

	int32 userId = 0;
	TSharedPtr<FString> outString = MakeShared<FString>();

	ErrNo errNo = WrapperDB::CreateAccount(strID, strPassword, outString);
	if (errNo.IsFailed())
	{
		controller->AddNotifyMessage(TEXT("계정 생성에 실패하였습니다."));
		return false;
	}

	userId = FCString::Atoi(**outString);

	URPGWorldGameInstance* instance = GetRPGWorldGameInstance();
	if (instance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("게임인스턴스 타입 변환 실패"));
		return false;
	}
	controller->SetUserID(userId);
	controller->SetUserName(id);

	TSubclassOf<APawn> pawnClass = controller->GetControllerPawnClass();

	APawn* pawn = GetWorld()->SpawnActor<APawn>(pawnClass, FVector(1750.f, 1750.f, 92.01f), FRotator::ZeroRotator);
	controller->Possess(pawn);

	return true;
}

bool ARPGWorldGameMode::ResponseLoginAccount(ABasePlayerController* controller, const FString& id, const FString& password)
{
	if (controller->IsValidLowLevel() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("유효하지 않은 컨트롤러 입니다."));
		return false;
	}

	if (id.IsEmpty() || password.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("비정상적인 아이디 : [%s] 혹은, 비밀번호 : [%s] 입니다."), *id, *password);
		return false;
	}

	const std::string strID = TCHAR_TO_UTF8(*id);
	const std::string strPassword = TCHAR_TO_UTF8(*password);

	int32 userId = 0;
	TSharedPtr<FString> outString = MakeShared<FString>();

	ErrNo errNo = WrapperDB::LoginAccount(strID, strPassword, outString);
	if (errNo.IsFailed())
	{
		controller->AddNotifyMessage(TEXT("로그인에 실패하였습니다."));
		return false;
	}

	userId = FCString::Atoi(**outString);

	URPGWorldGameInstance* instance = GetRPGWorldGameInstance();
	if (instance == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("게임인스턴스 타입 변환 실패"));
		return false;
	}
	controller->SetUserID(userId);
	controller->SetUserName(id);

	TSubclassOf<APawn> pawnClass = controller->GetControllerPawnClass();

	APawn* pawn = GetWorld()->SpawnActor<APawn>(pawnClass, /*FVector(260.f, 290.f, 92.01f)*/FVector(1750.f, 1750.f, 92.01f), FRotator::ZeroRotator);
	controller->Possess(pawn);

	return true;
}

void ARPGWorldGameMode::DeathBossMonster(ABaseBoss* boss, AController* lastDamageInstigator)
{
	ensureMsgfDebug(boss->IsDead() == true, TEXT("몬스터가 Dead상태가 아닙니다."));

	ABasePlayerController* playerController = Cast<ABasePlayerController>(lastDamageInstigator);
	if (playerController == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("컨트롤러를 플레이어 컨트롤러로 캐스트를 실패하였습니다."));
		return;
	}

	const FString& playerName = playerController->GetUserName();

	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		ABasePlayerController* constroller = Cast<ABasePlayerController>(iter->Get());
		if (iter->Get() == lastDamageInstigator)
		{
			constroller->AddNotifyMessage(FString::Printf(TEXT("보스 몬스터를 처치하였습니다."), *playerName), 3.0f);
		}
		else if (constroller)
		{
			constroller->AddNotifyMessage(FString::Printf(TEXT("%s님이 보스 몬스터를 처치하였습니다."), *playerName), 3.0f);
		}
	}

}

void ARPGWorldGameMode::CheatSpawnBoss()
{
	SpawnBoss();
}

void ARPGWorldGameMode::SpawnBoss()
{
	const DynamicMonsterInfo monsterInfo = GetInfoSubsystem(UMonsterInfoSubsystem)->GetInfo(BossMonsterKey);

	TSubclassOf<ABaseMonster> monsterClass(monsterInfo->_monsterClass.LoadSynchronous());

	const int32 randSpawnPosIndex = FMath::RandRange(0, _bossSpawnPosition.Num() - 1);
	const FVector spawnPos = _bossSpawnPosition[randSpawnPosIndex];

	const FTransform spawnTransform(FRotator::ZeroRotator, spawnPos, FVector::OneVector);

	ABaseMonster* monster = GetWorld()->SpawnActorDeferred<ABaseMonster>(monsterClass, spawnTransform);
	monster->SetMonsterKey(BossMonsterKey);
	monster->SetSpawnInfo(spawnTransform);
	monster->SetAIMoveRadius(monsterInfo->_aiMoveRadius);
	monster->SetAttackDamage(monsterInfo->_attackDamage);
	monster->SetMaxHealth(monsterInfo->_health);

	monster->FinishSpawning(spawnTransform);

	for (FConstPlayerControllerIterator iter = GetWorld()->GetPlayerControllerIterator(); iter; ++iter)
	{
		const float randXOffset = FMath::FRand() * 200.f - 100.f;
		const float randYOffset = FMath::FRand() * 200.f - 100.f;

		ABasePlayerController* constroller = Cast<ABasePlayerController>(iter->Get());
		if (constroller != nullptr)
		{
			constroller->CreateSpawnBossWidget(FVector(spawnPos.X + randXOffset, spawnPos.Y + randYOffset, spawnPos.Z), 10.f);
		}
	}

	const float nextSpawnDelay = GetSecondsNextHour();
	GetWorldTimerManager().SetTimer(_timerHandle, this, &ARPGWorldGameMode::SpawnBoss, nextSpawnDelay, false);
}

float ARPGWorldGameMode::GetSecondsNextHour()
{
	const FDateTime time = FDateTime::Now();
	const int32 seconds = time.GetMinute() * 60 + time.GetSecond();

	return 3600.f - seconds;
}

const DropItem* ARPGWorldGameMode::GetRandomDropItem(const TArray<DropItem>& dropItemArray)
{
	float randomValue = FMath::FRand();
	float currentProbability = 0.0f;

	for (const DropItem& dropItem : dropItemArray)
	{
		currentProbability += dropItem._probability;
		if (randomValue <= currentProbability)
		{
			return &dropItem;
		}
	}

	return nullptr;
}
