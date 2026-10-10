#include "UI/Portal/SignIn/ConfirmForgotPasswordPage.h"

#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UConfirmForgotPasswordPage::ClearTextBoxes()
{
	TextBox_Password->SetText(FText::GetEmpty());
	TextBox_ConfirmPassword->SetText(FText::GetEmpty());
	TextBox_ConfirmationCode->SetText(FText::GetEmpty());
	TextBlock_StatusMessage->SetText(FText::GetEmpty());
	Button_Confirm->SetIsEnabled(false);
}

void UConfirmForgotPasswordPage::NativeConstruct()
{
	Super::NativeConstruct();

	TextBox_Password->OnTextChanged.AddDynamic(this, &UConfirmForgotPasswordPage::UpdateConfirmButtonState);
	TextBox_ConfirmPassword->OnTextChanged.AddDynamic(this, &UConfirmForgotPasswordPage::UpdateConfirmButtonState);
	TextBox_ConfirmationCode->OnTextChanged.AddDynamic(this, &UConfirmForgotPasswordPage::UpdateConfirmButtonState);
	Button_Confirm->SetIsEnabled(false);
}

void UConfirmForgotPasswordPage::UpdateConfirmButtonState(const FText& Text)
{
	const bool bArePasswordsEqual = TextBox_Password->GetText().ToString() == TextBox_ConfirmPassword->GetText().ToString();
	const bool bIsPasswordLongEnough = TextBox_Password->GetText().ToString().Len() >= 8;
	const bool bValidConfirmationCode = IsValidConfirmationCode(TextBox_ConfirmationCode->GetText());

	FString StatusMessage;
	const bool bIsStrongPassword = IsStrongPassword(TextBox_Password->GetText().ToString(), StatusMessage);
	if (!bValidConfirmationCode)
	{
		TextBlock_StatusMessage->SetText(FText::FromString("Confirmation code must be 6 digits"));
	}
	else if (!bIsStrongPassword)
	{
		TextBlock_StatusMessage->SetText(FText::FromString(StatusMessage));
	}
	else if (!bArePasswordsEqual)
	{
		TextBlock_StatusMessage->SetText(FText::FromString("Please ensure that passwords match."));
	}
	else if (!bIsPasswordLongEnough)
	{
		TextBlock_StatusMessage->SetText(FText::FromString("Password must be at least 8 characters long."));
	}
	else
	{
		TextBlock_StatusMessage->SetText(FText::GetEmpty());
	}
	Button_Confirm->SetIsEnabled(bArePasswordsEqual && bIsStrongPassword && bValidConfirmationCode);
}

void UConfirmForgotPasswordPage::UpdateStatusMessage(const FString& StatusMessage, bool bShouldResetWidgets)
{
	TextBlock_StatusMessage->SetText(FText::FromString(StatusMessage));
	if (bShouldResetWidgets)
	{
		Button_Confirm->SetIsEnabled(true);
	}
}