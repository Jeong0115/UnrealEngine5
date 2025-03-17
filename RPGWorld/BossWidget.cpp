// Fill out your copyright notice in the Description page of Project Settings.


#include "BossWidget.h"

#include "Components\ProgressBar.h"
#include "Components\Button.h"

#include "BasePlayerController.h"

void UBossWidget::NativeConstruct()
{
	Super::NativeConstruct();

	_warpPoint = FVector::ZeroVector;

	_warpButton->OnClicked.AddDynamic(this, &UBossWidget::WarpPlayer);

	_timeProgressBar->SetPercent(1.0f);
}

void UBossWidget::NativeTick(const FGeometry& myGeometry, float inDeltaTime)
{
	Super::Tick(myGeometry, inDeltaTime);

	_remainLifeTime -= inDeltaTime;

	const float ratio = FMath::Clamp(_remainLifeTime / _lifeTime, 0.f, 1.0f);
	_timeProgressBar->SetPercent(ratio);

	if (_remainLifeTime <= 0.f)
	{
		RemoveFromParent();
	}
}

void UBossWidget::Init(const FVector& warpPoint, const float lifeTime)
{
	ensureMsgf(lifeTime > 0.f, TEXT("보스 위젯 유지 시간이 비정상 입니다. life time : [%f]"), lifeTime);

	_warpPoint = warpPoint;
	_lifeTime = _remainLifeTime = lifeTime;
}

void UBossWidget::WarpPlayer()
{
	if (_warpPoint == FVector::ZeroVector)
	{
		UE_LOG(LogTemp, Warning, TEXT("워프 포인트가 설정되지 않았습니다."));
		return;
	}

	ABasePlayerController* controller = Cast<ABasePlayerController>(GetOwningPlayer());
	controller->ReqWarpPlayer(_warpPoint);

	RemoveFromParent();
}
