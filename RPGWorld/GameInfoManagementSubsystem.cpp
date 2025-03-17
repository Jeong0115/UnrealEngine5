#include "GameInfoManagementSubsystem.h"

#include "TimerManager.h"
void UGameInfoManagementSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(_timerHandle, this, &UGameInfoManagementSubsystem::RemoveTemporaryInfo, 5.0f, true);
	}
}

void UGameInfoManagementSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGameInfoManagementSubsystem::RegisterDynamicInfo(TSharedPtr<FInfoStruct> info, const EInfoLifeSpan lifeSpan)
{
	if (GetWorld() == nullptr)
	{
		return;
	}

	if (_bClearInfoEveryTick == true)
	{
		_dynamicInfoArray[static_cast<int32>(EInfoLifeSpan::Temporary)].Emplace(info);

		return;
	}

	_dynamicInfoArray[static_cast<int32>(lifeSpan)].Emplace(info);
}

void UGameInfoManagementSubsystem::CheatClearInfoArray()
{
	for (DynamicInfoArray& dynamicInfoArray : _dynamicInfoArray)
	{
		dynamicInfoArray.Reset();
	}
}

void UGameInfoManagementSubsystem::CheatForceClearInfoEveryTick()
{
	_bClearInfoEveryTick = true;
}


void UGameInfoManagementSubsystem::RemoveTemporaryInfo()
{
	// 테스트용 사이즈 계산
	_gameInfoMemorySize = 0;

	for (const DynamicInfoArray& dynamicInfoArray : _dynamicInfoArray)
	{
		for (auto& dynamicInfo : dynamicInfoArray)
		{
			_gameInfoMemorySize += dynamicInfo->GetMemorySize();
		}
	}

	_dynamicInfoArray[static_cast<int32>(EInfoLifeSpan::Temporary)].Reset();

	// _dynamicInfoArray[static_cast<int32>(EInfoLifeSpan::Stage)]; 현재 스테이는 하나... 
	// _dynamicInfoArray[static_cast<int32>(EInfoLifeSpan::Manual)];
}
