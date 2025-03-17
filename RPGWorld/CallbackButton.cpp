#include "CallbackButton.h"

#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UCallbackButton::NativeConstruct()
{
	Super::NativeConstruct();
}

void UCallbackButton::BindButtonCallback(TFunction<void()> func)
{
	_callbackFunc = func;
	_button->OnClicked.AddDynamic(this, &UCallbackButton::OnButtonClicked);
}

void UCallbackButton::SetText(const FText& text)
{
	_text->SetText(text);
}

void UCallbackButton::SetImage(UTexture2D* texture)
{
	_image->SetBrushFromTexture(texture);
}

void UCallbackButton::OnButtonClicked()
{
	if (_callbackFunc)
	{
		_callbackFunc();
	}
}