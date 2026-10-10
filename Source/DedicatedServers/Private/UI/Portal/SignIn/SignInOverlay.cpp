#include "UI/Portal/SignIn/SignInOverlay.h"
#include "UI/API/GameSessions/JoinGame.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "UI/Portal/PortalManager.h"
#include "Components/WidgetSwitcher.h"
#include "Player/DSLocalPlayerSubsystem.h"
#include "UI/Portal/SignIn/ConfirmForgotPasswordPage.h"
#include "UI/Portal/SignIn/SignInPage.h"
#include "UI/Portal/SignIn/SignUpPage.h"
#include "UI/Portal/SignIn/ConfirmSignUpPage.h"
#include "UI/Portal/SignIn/ForgotPasswordPage.h"
#include "UI/Portal/SignIn/SuccessConfirmedPage.h"



void USignInOverlay::NativeConstruct()
{
	Super::NativeConstruct();
	check(PortalManagerClass);

	PortalManager = NewObject<UPortalManager>(this, PortalManagerClass);

	SignInPage->Button_SignIn->OnClicked.AddDynamic(this, &USignInOverlay::SignInButtonClicked);
	SignInPage->Button_SignUp->OnClicked.AddDynamic(this, &USignInOverlay::ShowSignUpPage);
	SignInPage->Button_Back->OnClicked.AddDynamic(PortalManager, &UPortalManager::GoToMainMenu);
	SignInPage->Button_ForgotPassword->OnClicked.AddDynamic(this, &USignInOverlay::ShowForgotPasswordPage);
	PortalManager->SignInStatusMessageDelegate.AddDynamic(SignInPage, &USignInPage::UpdateStatusMessage);

	
	SignUpPage->Button_Back->OnClicked.AddDynamic(this, &USignInOverlay::ShowSignInPage);
	SignUpPage->Button_SignUp->OnClicked.AddDynamic(this, &USignInOverlay::SignUpButtonClicked);
	PortalManager->SignUpStatusMessageDelegate.AddDynamic(SignUpPage, &USignUpPage::UpdateStatusMessage);
	PortalManager->OnSignUpSucceeded.AddDynamic(this, &USignInOverlay::OnSignUpSucceeded);
	
	ConfirmSignUpPage->Button_Back->OnClicked.AddDynamic(this, &USignInOverlay::ShowSignUpPage);
	ConfirmSignUpPage->Button_Confirm->OnClicked.AddDynamic(this, &USignInOverlay::ConfirmButtonClicked);
	PortalManager->OnConfirmSucceeded.AddDynamic(this, &USignInOverlay::OnConfirmSucceeded);
	PortalManager->ConfirmStatusMessageDelegate.AddDynamic(ConfirmSignUpPage, &UConfirmSignUpPage::UpdateStatusMessage);

	ForgotPasswordPage->Button_Back->OnClicked.AddDynamic(this, &USignInOverlay::ShowSignInPage);
	ForgotPasswordPage->Button_ResetPassword->OnClicked.AddDynamic(this, &USignInOverlay::OnResetPasswordButtonClicked);
	PortalManager->OnRequestForgotPasswordSucceeded.AddDynamic(this, &USignInOverlay::OnForgotPasswordRequestSucceeded);
	PortalManager->ForgotPasswordMessageDelegate.AddDynamic(ForgotPasswordPage, &UForgotPasswordPage::UpdateStatusMessage);

	ConfirmForgotPasswordPage->Button_Back->OnClicked.AddDynamic(this, &USignInOverlay::ShowForgotPasswordPage);
	ConfirmForgotPasswordPage->Button_Confirm->OnClicked.AddDynamic(this, &USignInOverlay::ConfirmForgotPasswrodButtonClicked);
	PortalManager->OnConfirmForgotPasswordSucceeded.AddDynamic(this,  &USignInOverlay::OnConfirmForgotPasswordSucceeded);
	PortalManager->ConfirmForgotPasswordMessageDelegate.AddDynamic(ConfirmForgotPasswordPage, &UConfirmForgotPasswordPage::UpdateStatusMessage);
	
	SuccessConfirmedPage->Button_Ok->OnClicked.AddDynamic(this, &USignInOverlay::ShowSignInPage);
	
	ShowSignInPage();
	//TryAutoSignIn();
	
}

bool USignInOverlay::TryAutoSignIn()
{
	if (UDSLocalPlayerSubsystem* DSLocalPlayerSubsystem = PortalManager->GetDSLocalPlayerSubsystem(); IsValid(DSLocalPlayerSubsystem))
	{
		const FString& Username = DSLocalPlayerSubsystem->Username;
		const FString& Password = DSLocalPlayerSubsystem->Password;
		if (Username.IsEmpty() || Password.IsEmpty()) return false;

		SignInPage->Button_SignIn->SetIsEnabled(false);
		PortalManager->SignIn(Username, Password);
		return true;
	}
	return false;
}
void USignInOverlay::OnSignInSucceeded()
{
	if (WidgetSwitcher->GetActiveWidget() != SignInPage)
	{
		return;
	}

	if (SignInPage->CheckBox_RememberMe->IsChecked())
	{
		SignInPage->SaveRememberedCreds();
	}
	else
	{
		SignInPage->ClearRememberedCreds();
	}
}
void USignInOverlay::ShowSignInPage()
{
	SignInPage->Button_SignIn->SetIsEnabled(true);
	WidgetSwitcher->SetActiveWidget(SignInPage);
}

void USignInOverlay::ShowSignUpPage()
{
	WidgetSwitcher->SetActiveWidget(SignUpPage);
}

void USignInOverlay::ShowConfirmSignUpPage()
{
	WidgetSwitcher->SetActiveWidget(ConfirmSignUpPage);
}

void USignInOverlay::ShowSuccessConfirmedPage()
{
	WidgetSwitcher->SetActiveWidget(SuccessConfirmedPage);
}

void USignInOverlay::ShowForgotPasswordPage()
{
	WidgetSwitcher->SetActiveWidget(ForgotPasswordPage);
}

void USignInOverlay::ShowConfirmForgotPasswordPage()
{
	WidgetSwitcher->SetActiveWidget(ConfirmForgotPasswordPage);
}

void USignInOverlay::SignInButtonClicked()
{
	const FString Username = SignInPage->TextBox_UserName->GetText().ToString();
	const FString Password = SignInPage->TextBox_Password->GetText().ToString();
	if (UDSLocalPlayerSubsystem* DSLocalPlayerSubsystem = PortalManager->GetDSLocalPlayerSubsystem(); IsValid(DSLocalPlayerSubsystem))
	{
		DSLocalPlayerSubsystem->Password = Password;
	}
	PortalManager->SignIn(Username, Password);
}

void USignInOverlay::SignUpButtonClicked()
{
	const FString Username = SignUpPage->TextBox_UserName->GetText().ToString();
	const FString Password = SignUpPage->TextBox_Password->GetText().ToString();
	const FString Email = SignUpPage->TextBox_Email->GetText().ToString();
	PortalManager->SignUp(Username, Password, Email);
}

void USignInOverlay::ConfirmButtonClicked()
{
	const FString ConfirmationCode = ConfirmSignUpPage->TextBox_ConfirmationCode->GetText().ToString();
	ConfirmSignUpPage->Button_Confirm->SetIsEnabled(false);
	PortalManager->Confirm(ConfirmationCode);
}

void USignInOverlay::OnSignUpSucceeded()
{
	SignUpPage->ClearTextBoxes();
	ConfirmSignUpPage->TextBlock_Destination->SetText(FText::FromString(PortalManager->LastSignUpResponse.CodeDeliveryDetails.Destination));
	ShowConfirmSignUpPage();
}

void USignInOverlay::OnConfirmSucceeded()
{
	ConfirmSignUpPage->ClearTextBoxes();
	ShowSuccessConfirmedPage();
}

void USignInOverlay::OnConfirmForgotPasswordSucceeded()
{
	ConfirmForgotPasswordPage->ClearTextBoxes();
	ShowSuccessConfirmedPage();
}

void USignInOverlay::OnResetPasswordButtonClicked()
{
	ForgotPasswordPage->Button_ResetPassword->SetIsEnabled(false);
	const FString Username = ForgotPasswordPage->TextBox_UserName->GetText().ToString();
	PortalManager->RequestForgotPassword(Username);
}

void USignInOverlay::OnForgotPasswordRequestSucceeded()
{
	ForgotPasswordPage->ClearTextBoxes();
	ConfirmForgotPasswordPage->TextBlock_Destination->SetText(FText::FromString(PortalManager->LastForgotPasswordResponse.CodeDeliveryDetails.Destination));
	ShowConfirmForgotPasswordPage();
}

void USignInOverlay::ConfirmForgotPasswrodButtonClicked()
{
	const FString ConfirmationCode = ConfirmForgotPasswordPage->TextBox_ConfirmationCode->GetText().ToString();
	const FString Password = ConfirmForgotPasswordPage->TextBox_Password->GetText().ToString();
	ConfirmForgotPasswordPage->Button_Confirm->SetIsEnabled(false);
	PortalManager->ConfirmForgotPassword(ConfirmationCode, Password);
}
