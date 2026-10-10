#include "UI/Portal/SignIn/ConfirmSignUpPage.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UConfirmSignUpPage::ClearTextBoxes()
{
	TextBlock_Destination->SetText(FText::GetEmpty());
	TextBlock_StatusMessage->SetText(FText::GetEmpty());
	TextBox_ConfirmationCode->SetText(FText::GetEmpty());
}

void UConfirmSignUpPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		Button_Confirm->SetIsEnabled(true);
	}
}

void UConfirmSignUpPage::NativeConstruct()
{
	Super::NativeConstruct();
	TextBox_ConfirmationCode->OnTextChanged.AddDynamic(this, &UConfirmSignUpPage::UpdateConfirmButtonState);
	Button_Confirm->SetIsEnabled(false);
}

void UConfirmSignUpPage::UpdateConfirmButtonState(const FText& Text)
{
	const FRegexPattern SixDigitsPattern(TEXT(R"(^\d{6}$)"));
	FRegexMatcher Matcher(SixDigitsPattern, Text.ToString());

	const bool bValidConfirmationCode = Matcher.FindNext();

	Button_Confirm->SetIsEnabled(bValidConfirmationCode);

	if (!bValidConfirmationCode)
	{
		TextBlock_StatusMessage->SetText(FText::FromString("Confirmation code must be 6 digits"));
	}
	else
	{
		TextBlock_StatusMessage->SetText(FText::GetEmpty());
	}
}
