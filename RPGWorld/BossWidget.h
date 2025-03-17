#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BossWidget.generated.h"

class UProgressBar;
class UButton;

UCLASS()
class RPGWORLD_API UBossWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void	NativeConstruct() override;
	virtual void	NativeTick(const FGeometry& myGeometry, float inDeltaTime) override;

	void			Init(const FVector& warpPoint, const float lifeTime);

private:
	UFUNCTION()
	void			WarpPlayer();

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UProgressBar*	_timeProgressBar;

	UPROPERTY(EditDefaultsOnly, meta = (BindWidget))
	UButton*		_warpButton;

	FVector			_warpPoint;

	float			_lifeTime;
	float			_remainLifeTime;
};
