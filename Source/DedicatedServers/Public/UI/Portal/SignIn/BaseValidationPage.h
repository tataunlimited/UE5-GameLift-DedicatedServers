#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BaseValidationPage.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UBaseValidationPage : public UUserWidget
{
	GENERATED_BODY()

protected:

	UFUNCTION()
	bool IsStrongPassword(const FString& Password, FString& StatusMessage);

	UFUNCTION()
	bool IsValidEmail(const FString& Email);

	UFUNCTION()
	bool IsValidConfirmationCode(const FText& ConfirmationCode);
};
