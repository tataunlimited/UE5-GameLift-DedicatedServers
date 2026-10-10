#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ForgotPasswordPage.generated.h"

class UButton;
class UTextBlock;
class UEditableTextBox;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UForgotPasswordPage : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UEditableTextBox> TextBox_UserName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_ResetPassword;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Back;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;

	UFUNCTION()
	void UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets);

	void ClearTextBoxes();

protected:
	virtual void NativeConstruct() override;

private:
	
	UFUNCTION()
	void UpdateResetButtonState(const FText& Text);

};
