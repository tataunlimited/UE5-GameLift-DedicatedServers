#include "UI/Portal/PortalManager.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Data/API/APIData.h"
#include "GameplayTags/DedicatedServerTags.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSLocalPlayerSubsystem.h"
#include "UI/Interfaces/HUDManagement.h"
#include "GameFramework/HUD.h"

void UPortalManager::SignIn(const FString& Username, const FString& Password)
{
	SignInStatusMessageDelegate.Broadcast("Signing in...", false);
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::SignIn_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::SignIn);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	LastUsername = Username;
	
	TMap<FString, FString> Params = {
		{TEXT("username"), Username},
		{TEXT("password"), Password}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}
void UPortalManager::SignIn_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		SignInStatusMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			SignInStatusMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}

		FDSInititateAuthResponse InitiateAuthResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &InitiateAuthResponse);
		InitiateAuthResponse.Dump();
		
		UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetDSLocalPlayerSubsystem();
		if (IsValid(LocalPlayerSubsystem))
		{
			LocalPlayerSubsystem->InitializeTokens(InitiateAuthResponse.AuthenticationResult, this);
			LocalPlayerSubsystem->Username = LastUsername;
			LocalPlayerSubsystem->Email = InitiateAuthResponse.Email;
		}

		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (IsValid(LocalPlayerController))
		{
			if (IHUDManagement* HUDManagement = Cast<IHUDManagement>(LocalPlayerController->GetHUD()))
			{
				HUDManagement->OnSignIn();
			}
		}
	}
}

void UPortalManager::SignUp(const FString& Username, const FString& Password, const FString& Email)
{
	SignUpStatusMessageDelegate.Broadcast("Signing up...", false);
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::SignUp_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::SignUp);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	LastUsername = Username;
	
	TMap<FString, FString> Params = {
		{TEXT("username"), Username},
		{TEXT("password"), Password},
		{TEXT("email"), Email}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UPortalManager::SignUp_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		SignUpStatusMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			SignUpStatusMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &LastSignUpResponse);
		OnSignUpSucceeded.Broadcast();
	}
}



void UPortalManager::Confirm(const FString& ConfirmationCode)
{
	check(APIData);
	ConfirmStatusMessageDelegate.Broadcast(TEXT("Checking verification code..."), false);

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::Confirm_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::ConfirmSignUp);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("Put"));
	Request->SetHeader("Content-Type", "application/json");

	TMap<FString, FString> Params = {
		{TEXT("confirmationCode"), ConfirmationCode},
		{TEXT("username"), LastUsername}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UPortalManager::Confirm_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ConfirmStatusMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			if (JsonObject->HasField(TEXT("name")))
			{
				FString Exception = JsonObject->GetStringField(TEXT("name"));
				if (Exception.Equals(TEXT("CodeMismatchException")))
				{
					ConfirmStatusMessageDelegate.Broadcast(TEXT("Incorrect verification code"), true);
					return;
				}
			}
			ConfirmStatusMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}
		OnConfirmSucceeded.Broadcast();
	}
}

void UPortalManager::GoToMainMenu()
{
	UGameplayStatics::OpenLevelBySoftObjectPtr(this, MainMenuMap);
}

void UPortalManager::RefreshTokens(const FString& RefreshToken)
{
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::RefreshToken_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::SignIn);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	
	TMap<FString, FString> Params = {
		{TEXT("refreshToken"), RefreshToken}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}


void UPortalManager::RefreshToken_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	IHUDManagement* HUDManagement = nullptr;
	if (IsValid(LocalPlayerController))
	{
		HUDManagement = Cast<IHUDManagement>(LocalPlayerController->GetHUD());
	}
	
	if (!bWasSuccessful)
	{
		if (HUDManagement)
		{
			HUDManagement->OnTokensRefreshed(false);	
		}
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			if (HUDManagement)
			{
				HUDManagement->OnTokensRefreshed(false);	
			}
			return;
		}
		
		FDSInititateAuthResponse InitiateAuthResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &InitiateAuthResponse);

		UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetDSLocalPlayerSubsystem();
		if (IsValid(LocalPlayerSubsystem))
		{
			LocalPlayerSubsystem->UpdateTokens(InitiateAuthResponse.AuthenticationResult.AccessToken, InitiateAuthResponse.AuthenticationResult.IdToken);
		}
		if (HUDManagement)
		{
			HUDManagement->OnTokensRefreshed(true);
		}
		return;
	}
	if (HUDManagement)
	{
		HUDManagement->OnTokensRefreshed(false);
	}
}

void UPortalManager::SignOut(const FString& AccessToken)
{
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::SignOut_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::SignOut);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	
	TMap<FString, FString> Params = {
		{TEXT("accessToken"), AccessToken}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}



void UPortalManager::SignOut_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			return;
		}
	}

	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetDSLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem))
	{
		LocalPlayerSubsystem->Username = "";
		LocalPlayerSubsystem->Email = "";
		LocalPlayerSubsystem->Password = "";
	}
	
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (IsValid(LocalPlayerController))
	{
		if (IHUDManagement* HUDManagement = Cast<IHUDManagement>(LocalPlayerController->GetHUD()))
		{
			HUDManagement->OnSignOut();
		}
	}
}




void UPortalManager::RequestForgotPassword(const FString& Username)
{
	ForgotPasswordMessageDelegate.Broadcast("Requesting forgot password...", false);
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::RequestForgotPassword_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::ForgotPassword);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	LastUsername = Username;
	
	TMap<FString, FString> Params = {
		{TEXT("username"), Username},
		{TEXT("email"), ""}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UPortalManager::RequestForgotPassword_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ForgotPasswordMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			ForgotPasswordMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &LastForgotPasswordResponse);
		OnRequestForgotPasswordSucceeded.Broadcast();
	}
}

void UPortalManager::ConfirmForgotPassword(const FString& ConfirmationCode, const FString& Password)
{
	ConfirmForgotPasswordMessageDelegate.Broadcast("Confirming forgot password...", false);
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UPortalManager::ConfirmForgotPassword_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PortalAPI::ConfirmForgotPassword);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	
	TMap<FString, FString> Params = {
		{TEXT("username"), LastUsername},
		{TEXT("password"), Password},
		{TEXT("confirmationCode"), ConfirmationCode}
		
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UPortalManager::ConfirmForgotPassword_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ConfirmForgotPasswordMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			ConfirmForgotPasswordMessageDelegate.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}
		OnConfirmForgotPasswordSucceeded.Broadcast();
	}
}
