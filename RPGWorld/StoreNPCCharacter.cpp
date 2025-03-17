#include "StoreNPCCharacter.h"

#include "Camera\CameraActor.h"
#include "Components\WidgetComponent.h"
#include "Kismet\GameplayStatics.h"

#include "BasePlayerController.h"
#include "CallbackButton.h"

#include "StoreWidget.h"

AStoreNPCCharacter::AStoreNPCCharacter()
	: _storeKeyRaw(-1)
{
	PrimaryActorTick.bCanEverTick = true;

	_widgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("Widget"));
	_widgetComponent->SetupAttachment(RootComponent);
}

void AStoreNPCCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		return;
	}

	if (_buttonWidgetClass == nullptr)
	{
		return;
	}

	_storeCamera = GetWorld()->SpawnActor<ACameraActor>();
	_storeCamera->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
	_storeCamera->SetActorRelativeTransform(_cameraTransform);

	_widgetComponent->SetWidgetClass(_buttonWidgetClass);
	_widgetComponent->SetOwnerPlayer(UGameplayStatics::GetPlayerController(this, 0)->GetLocalPlayer());

	UCallbackButton* widget = Cast<UCallbackButton>(_widgetComponent->GetWidget());
	widget->SetText(_widgetText);
	widget->SetImage(_widgetTexture);

	widget->BindButtonCallback([this] {OpenStore(); });
}

void AStoreNPCCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AStoreNPCCharacter::OpenStore()
{
	if (HasAuthority() == true)
	{
		return;
	}

	APlayerController* playerController = UGameplayStatics::GetPlayerController(this, 0);
	playerController->SetViewTargetWithBlend(_storeCamera, 0.5f);
	Cast<ABasePlayerController>(playerController)->OpenStore(_storeKeyRaw);

	PlayOpenStoreAnimation();
}

void AStoreNPCCharacter::PlayOpenStoreAnimation_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("블루프린트에서 해당 함수를 구현해야 됩니다."));
}
