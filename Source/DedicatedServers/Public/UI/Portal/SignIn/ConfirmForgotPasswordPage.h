#pragma once

#include "CoreMinimal.h"
#include "BaseValidationPage.h"
#include "ConfirmForgotPasswordPage.generated.h"

class UEditableTextBox;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UConfirmForgotPasswordPage : public UBaseValidationPage
{
	GENERATED_BODY()

public:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_ConfirmationCode;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_Password;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_ConfirmPassword;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Destination;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Confirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	void ClearTextBoxes();
	
	UFUNCTION()
	void UpdateStatusMessage(const FString& StatusMessage, bool bShouldResetWidgets);
	
protected:
	virtual void NativeConstruct() override;


private:

	UFUNCTION()
	void UpdateConfirmButtonState(const FText& Text);


	
	
};


