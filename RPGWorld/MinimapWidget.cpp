#include "MinimapWidget.h"

#include "Blueprint/SlateBlueprintLibrary.h"

#include "BaseCharacter.h"
#include "MinimapSubsystem.h"


int32 UMinimapWidget::NativePaint(	const FPaintArgs& args, 
									const FGeometry& allottedGeometry, 
									const FSlateRect& myCullingRect, 
									FSlateWindowElementList& outDrawElements, 
									int32 layerId, 
									const FWidgetStyle& inWidgetStyle, 
									bool bParentEnabled) const
{
	const int32 returnLayerId = Super::NativePaint(args, allottedGeometry, myCullingRect, outDrawElements, layerId, inWidgetStyle, bParentEnabled);

	UGameInstance* gameInstance = GetGameInstance();
	if (gameInstance == nullptr)
	{
		return returnLayerId;
	}
	
	UMinimapSubsystem* minimapSubsystem = gameInstance->GetSubsystem<UMinimapSubsystem>();
	if (minimapSubsystem == nullptr)
	{
		return returnLayerId;
	}


	const TArray<ABaseCharacter*>& monsterArray = minimapSubsystem->GetDrawCharacterArray(ECharacterType::Monster);
	DrawCharacterIcon(monsterArray, FLinearColor::Red, allottedGeometry, outDrawElements, returnLayerId);

	const TArray<ABaseCharacter*>& otherPlayerArray = minimapSubsystem->GetDrawCharacterArray(ECharacterType::OtherPlayer);
	DrawCharacterIcon(otherPlayerArray, FLinearColor::Yellow, allottedGeometry, outDrawElements, returnLayerId);

	ABaseCharacter* controlledPlayer = Cast<ABaseCharacter>(GetOwningPlayer()->GetPawn());
	DrawCharacterIcon({ controlledPlayer }, FLinearColor::Green, allottedGeometry, outDrawElements, returnLayerId);

	return returnLayerId;
}

void UMinimapWidget::DrawCharacterIcon(	const TArray<ABaseCharacter*>& characterArray, 
										const FLinearColor& drawColor, 
										const FGeometry& allottedGeometry, 
										FSlateWindowElementList& drawElements, 
										const int32 layerId) const
{
	FSlateColorBrush brush(FLinearColor::White);

	for (ABaseCharacter* character : characterArray)
	{
		if (character->IsDead() == true)
		{
			continue;
		}

		const FVector location = character->GetActorLocation();

		// 스테이지 하나여서 하드 코딩으로 스테이지 사이즈 노말라이즈
		float normalX = (location.Y + 3500.f) / 10500.f;
		float normalY = 1.0f - ((location.X + 3500.f) / 10500.f);

		FVector2D localPosition(normalX, normalY);
		localPosition *= _minimapSize;
		localPosition += _minimapOffset;	// 미내맵 테두리
		localPosition -= _drawSize / 2.f;	// 아이콘 사이즈 (좌표가 좌상단으로 잡혀서 중앙으로 옮기기 위해)

		FSlateLayoutTransform DebugTransform(localPosition);

		FSlateDrawElement::MakeBox(
			drawElements,
			layerId,
			allottedGeometry.ToPaintGeometry(_drawSize, DebugTransform),
			&brush,
			ESlateDrawEffect::None,
			drawColor
		);
	}
}
