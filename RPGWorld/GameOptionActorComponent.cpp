#include "GameOptionActorComponent.h"

#if WITH_SERVER_CODE
#include "DB\Public\WrapperDBGameOption.h"
#endif

UGameOptionActorComponent::UGameOptionActorComponent()
{
}

void UGameOptionActorComponent::BeginPlay()
{
	Super::BeginPlay();

	SetIsReplicated(true);
}

void UGameOptionActorComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGameOptionActorComponent, _currentInventorySize);
	DOREPLIFETIME(UGameOptionActorComponent, _maxInventorySize);
}

void UGameOptionActorComponent::LoadDataFromDB()
{
#if WITH_SERVER_CODE

	TArray<FGameOptionTable> gameOptionTableArray;
	ErrNo errNo = WrapperDB::GetPlayerGameOption(gameOptionTableArray, GetUserID());
	if (errNo.IsFailed() == true)
	{
		UE_LOG(LogTemp, Error, TEXT("DB에서 GameOption데이터를 얻어오는데 실패하였습니다. : [%s]"), *errNo.GetText());
		return;
	}

	_currentInventorySize = gameOptionTableArray[0]._currentInventorySize;

#endif

}

void UGameOptionActorComponent::ReqExtendInventoryClient_Implementation()
{
	// 대충 골드 확인
	// 기타 조건 확인
	// 맞으면 서버

	SendRPC(Server, ExtendInventory);
}

void UGameOptionActorComponent::ResponseExtendInventoryServer_Implementation()
{
	// 여기서도 추가 검증?
	// DB도 찍고
	_currentInventorySize += 4;
}
