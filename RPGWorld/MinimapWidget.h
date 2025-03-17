// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MinimapWidget.generated.h"

class ABaseCharacter;

UCLASS()
class RPGWORLD_API UMinimapWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual int32	NativePaint(const FPaintArgs& args, const FGeometry& allottedGeometry, const FSlateRect& myCullingRect, 
								FSlateWindowElementList& outDrawElements, int32 layerId, const FWidgetStyle& inWidgetStyle, bool bParentEnabled) const override;

private:
	void			DrawCharacterIcon(const TArray<ABaseCharacter*>& characterArray, const FLinearColor& drawColor, const FGeometry& allottedGeometry, 
										FSlateWindowElementList& drawElements, const int32 layerId) const;

	UPROPERTY(EditDefaultsOnly)
	FVector2D		_minimapSize;

	UPROPERTY(EditDefaultsOnly)
	FVector2D		_minimapOffset;

	UPROPERTY(EditDefaultsOnly)
	FVector2D		_drawSize;

};
