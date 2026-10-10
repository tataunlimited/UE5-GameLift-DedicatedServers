#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/PortalManagement.h"
#include "UI/HTTP/HTTPRequestManager.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "PortalManager.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UPortalManager : public UHTTPRequestManager, public IPortalManagement
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage SignUpStatusMessageDelegate;

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage ConfirmStatusMessageDelegate;

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage SignInStatusMessageDelegate;

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage ForgotPasswordMessageDelegate;

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage ConfirmForgotPasswordMessageDelegate;

	UPROPERTY(BlueprintAssignable)
	FOnAPIRequestSucceeded OnSignUpSucceeded;

	UPROPERTY(BlueprintAssignable)
	FOnAPIRequestSucceeded OnConfirmSucceeded;

	UPROPERTY(BlueprintAssignable)
	FOnAPIRequestSucceeded OnRequestForgotPasswordSucceeded;

	UPROPERTY(BlueprintAssignable)
	FOnAPIRequestSucceeded OnConfirmForgotPasswordSucceeded;
	
	void SignIn(const FString& Username, const FString& Password);
	void SignUp(const FString& Username, const FString& Password, const FString& Email);
	void Confirm(const FString& ConfirmationCode);
	void SignOut(const FString& AccessToken);
	void RequestForgotPassword(const FString& Username);
	void ConfirmForgotPassword(const FString& ConfirmationCode, const FString& Password);

	// IPortalManagement
	virtual void RefreshTokens(const FString& RefreshToken) override;

	UFUNCTION()
	void GoToMainMenu();

	FDSSignUpResponse LastSignUpResponse;
	FDSForgotPasswordResponse LastForgotPasswordResponse;
	FString LastUsername;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> MainMenuMap;

private:

	void SignUp_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void Confirm_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SignIn_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void RefreshToken_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void SignOut_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void RequestForgotPassword_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void ConfirmForgotPassword_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	

};


