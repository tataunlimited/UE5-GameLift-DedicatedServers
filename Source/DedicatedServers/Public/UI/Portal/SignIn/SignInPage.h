#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SignInPage.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
class UCheckBox;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API USignInPage : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_UserName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_Password;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SignIn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_SignUp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ForgotPassword;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_RememberMe;

	UFUNCTION(BlueprintCallable, Category="Auth")
	void SaveRememberedCreds();

	UFUNCTION(BlueprintCallable, Category="Auth")
	void ClearRememberedCreds();

	UFUNCTION()
	void UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets);

protected:
	virtual void NativeConstruct() override;

private:
	void LoadRememberedCreds();

	// Obfuscation helpers (replace with yours if already implemented elsewhere)
	static FString Obfuscate(const FString& In);
	static FString Deobfuscate(const FString& In);

	static constexpr const TCHAR* RememberSlot = TEXT("Signin_Remember_Slot");
	static const int32 RememberUserIndex = 0;
	
};
