
#include "UI/Portal/SignIn/ForgotPasswordPage.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"

void UForgotPasswordPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		Button_ResetPassword->SetIsEnabled(true);
	}
}

void UForgotPasswordPage::ClearTextBoxes()
{
	TextBlock_StatusMessage->SetText(FText::GetEmpty());
	TextBox_UserName->SetText(FText::GetEmpty());
}

void UForgotPasswordPage::NativeConstruct()
{
	Super::NativeConstruct();
	TextBox_UserName->OnTextChanged.AddDynamic(this, &UForgotPasswordPage::UpdateResetButtonState);
	Button_ResetPassword->SetIsEnabled(false);
	
}

void UForgotPasswordPage::UpdateResetButtonState(const FText& Text)
{
	const bool bIsUsernameValid = !TextBox_UserName->GetText().IsEmpty() && !TextBox_UserName->GetText().ToString().Contains(" ");

	if (!bIsUsernameValid)
	{
		TextBlock_StatusMessage->SetText(FText::FromString("Please enter a valid Username."));
	}
	else
	{
		TextBlock_StatusMessage->SetText(FText::GetEmpty());
	}
	Button_ResetPassword->SetIsEnabled(bIsUsernameValid);

}
