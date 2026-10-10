#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SignInOverlay.generated.h"

class UConfirmForgotPasswordPage;
class UForgotPasswordPage;
class USuccessConfirmedPage;
class UConfirmSignUpPage;
class USignUpPage;
class USignInPage;
class UPortalManager;
class UJoinGame;
class UWidgetSwitcher;
class UButton;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API USignInOverlay : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPortalManager> PortalManagerClass;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UWidgetSwitcher> WidgetSwitcher;

	void OnSignInSucceeded();

protected:
	UFUNCTION()
	virtual void NativeConstruct() override;

private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USignInPage> SignInPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USignUpPage> SignUpPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UConfirmSignUpPage> ConfirmSignUpPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USuccessConfirmedPage> SuccessConfirmedPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UForgotPasswordPage> ForgotPasswordPage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UConfirmForgotPasswordPage> ConfirmForgotPasswordPage;
	
	UPROPERTY()
	TObjectPtr<UPortalManager> PortalManager;

	bool TryAutoSignIn();

	UFUNCTION()
	void ShowSignInPage();

	UFUNCTION()
	void ShowSignUpPage();

	UFUNCTION()
	void ShowConfirmSignUpPage();

	UFUNCTION()
	void ShowSuccessConfirmedPage();
	
	UFUNCTION()
	void ShowForgotPasswordPage();

	UFUNCTION()
	void ShowConfirmForgotPasswordPage();
	
	UFUNCTION()
	void SignInButtonClicked();

	UFUNCTION()
	void SignUpButtonClicked();

	UFUNCTION()
	void ConfirmButtonClicked();

	UFUNCTION()
	void OnSignUpSucceeded();

	UFUNCTION()
	void OnConfirmSucceeded();

	UFUNCTION()
	void OnConfirmForgotPasswordSucceeded();

	UFUNCTION()
	void OnResetPasswordButtonClicked();

	UFUNCTION()
	void OnForgotPasswordRequestSucceeded();
	
	UFUNCTION()
	void ConfirmForgotPasswrodButtonClicked();

};
