#include "Save/ServerSaveManager.h"
#include "Save/Helper/SaveSystemHelpers.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Data/API/APIData.h"
#include "GameplayTags/DedicatedServerTags.h"
#include "DedicatedServers/DedicatedServers.h"


// --- PUBLIC ENTRY POINTS ---

void UServerSaveManager::Server_SavePlayerStateToS3(USaveGame* PlayerSaveObject, const FString& Username)
{
	if (ActiveOperations.Contains(Username))
	{
		UE_LOG(LogDedicatedServers, Warning, TEXT("Server_SavePlayerStateToS3: Save operation already in progress for user %s."), *Username);
		return;
	}
	if (!IsValid(PlayerSaveObject))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("Server_SavePlayerStateToS3: PlayerSaveObject was null for user %s."), *Username);
		OnServerSaveToS3Complete.Broadcast(false, Username);
		return;
	}

	FServerAsyncOperationData& OpData = ActiveOperations.Add(Username);
	OpData.Username = Username;
	OpData.PlayerSerializedData = USaveSystemHelpers::SerializeSaveGameObject(PlayerSaveObject);
	
	if (OpData.PlayerSerializedData.Num() == 0)
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("Server_SavePlayerStateToS3: Serialization failed for user %s."), *Username);
		ActiveOperations.Remove(Username);
		OnServerSaveToS3Complete.Broadcast(false, Username);
		return;
	}

	RequestServerUploadURL(Username);
}

void UServerSaveManager::Server_LoadPlayerStateFromS3(const FString& Username)
{
	if (ActiveOperations.Contains(Username))
	{
		UE_LOG(LogDedicatedServers, Warning, TEXT("Server_LoadPlayerStateFromS3: Load operation already in progress for user %s."), *Username);
		return;
	}

	FServerAsyncOperationData& OpData = ActiveOperations.Add(Username);
	OpData.Username = Username;

	RequestServerDownloadURL(Username);
}

void UServerSaveManager::TriggerBlueprintSaveForPlayer(const FString& Username, APawn* Player)
{
	CreateAndSavePlayerData(Username, Player);
}

// --- OVERRIDDEN S3 HANDLERS ---

void UServerSaveManager::HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const FString* UsernamePtr = RequestUsernameMap.Find(Request);
	if (!UsernamePtr) return;
	FString Username = *UsernamePtr;
	RequestUsernameMap.Remove(Request);

	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("S3 upload failed for user %s."), *Username);
		ActiveOperations.Remove(Username);
		OnServerSaveToS3Complete.Broadcast(false, Username);
		return;
	}

	// S3 upload was successful, now confirm it with our backend.
	ConfirmServerS3Upload(Username);
}

void UServerSaveManager::HandleDownloadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const FString* UsernamePtr = RequestUsernameMap.Find(Request);
	if (!UsernamePtr) return;
	FString Username = *UsernamePtr;
	RequestUsernameMap.Remove(Request);

	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("S3 download failed for user %s."), *Username);
		ActiveOperations.Remove(Username);
		OnServerLoadFromS3Complete.Broadcast(false, nullptr, Username);
		return;
	}
	
	const TArray<uint8>& DownloadedData = Response->GetContent();
	USaveGame* LoadedPlayerSave = USaveSystemHelpers::DeserializeSaveGameObject(PlayerSaveGameClass, DownloadedData);
	
	// Broadcast result and clean up the operation data.
	ActiveOperations.Remove(Username);

	OnServerLoadFromS3Complete.Broadcast(IsValid(LoadedPlayerSave), LoadedPlayerSave, Username);
}


// --- SERVER SAVE PROCESS ---

void UServerSaveManager::RequestServerUploadURL(const FString& Username)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("saveType"), "player");
    
    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	RequestUsernameMap.Add(Request, Username); // Map this request to the user
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PlayerSaveAPI::ServerRequestUploadURL);
	Request->OnProcessRequestComplete().BindUObject(this, &UServerSaveManager::OnRequestServerUploadURLResponse);
	Request->SetURL(APIUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonPayload);
	Request->ProcessRequest();
}

void UServerSaveManager::OnRequestServerUploadURLResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const FString* UsernamePtr = RequestUsernameMap.Find(Request);
	if (!UsernamePtr) return;
	FString Username = *UsernamePtr;
	
	FServerAsyncOperationData* OpData = ActiveOperations.Find(Username);
	if (!OpData)
	{
		RequestUsernameMap.Remove(Request);
		return;
	}

	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("RequestServerUploadURL failed for user %s."), *Username);
		ActiveOperations.Remove(Username);
		RequestUsernameMap.Remove(Request);
		OnServerSaveToS3Complete.Broadcast(false, Username);
		return;
	}

	TSharedPtr<FJsonObject> OuterJsonObject;
    TSharedRef<TJsonReader<>> OuterReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
    if (FJsonSerializer::Deserialize(OuterReader, OuterJsonObject) && OuterJsonObject.IsValid())
    {
        FString BodyString;
        if (OuterJsonObject->TryGetStringField(TEXT("body"), BodyString))
        {
            TSharedPtr<FJsonObject> InnerJsonObject;
            TSharedRef<TJsonReader<>> InnerReader = TJsonReaderFactory<>::Create(BodyString);

        	if (FJsonSerializer::Deserialize(InnerReader, InnerJsonObject) && InnerJsonObject.IsValid())
        	{
        		FString UploadURL, S3Key;
        		if (InnerJsonObject->TryGetStringField(TEXT("uploadUrl"), UploadURL) && InnerJsonObject->TryGetStringField(TEXT("s3Key"), S3Key))
        		{
        			OpData->PlayerS3Key = S3Key;
        			RequestUsernameMap.Remove(Request); // Clean up the old API request mapping
        			TSharedRef<IHttpRequest> S3Request = UploadDataToS3(UploadURL, OpData->PlayerSerializedData);
        			RequestUsernameMap.Add(S3Request, Username); // Map the NEW S3 request to the user

        			return;
        		}
        	}
        }
    }


	
	UE_LOG(LogDedicatedServers, Error, TEXT("OnRequestServerUploadURLResponse: Failed to parse response for user %s."), *Username);
    ActiveOperations.Remove(Username);
    RequestUsernameMap.Remove(Request);
    OnServerSaveToS3Complete.Broadcast(false, Username);
}

void UServerSaveManager::ConfirmServerS3Upload(const FString& Username)
{
	FServerAsyncOperationData* OpData = ActiveOperations.Find(Username);
	if (!OpData) return;

    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("username"), Username);
    JsonObject->SetStringField(TEXT("playerS3Key"), OpData->PlayerS3Key);

    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	RequestUsernameMap.Add(Request, Username); // Map this request to the user
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PlayerSaveAPI::ServerConfirmSave);
	Request->OnProcessRequestComplete().BindUObject(this, &UServerSaveManager::OnConfirmServerS3UploadResponse);
	Request->SetURL(APIUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonPayload);
	Request->ProcessRequest();
}

void UServerSaveManager::OnConfirmServerS3UploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	const FString* UsernamePtr = RequestUsernameMap.Find(Request);
	if (!UsernamePtr) return;
	FString Username = *UsernamePtr;

	// This is the final step, so we can now clean up the operation data.
	ActiveOperations.Remove(Username);
	RequestUsernameMap.Remove(Request);

	bool bSuccess = bWasSuccessful && Response.IsValid() && EHttpResponseCodes::IsOk(Response->GetResponseCode());
	OnServerSaveToS3Complete.Broadcast(bSuccess, Username);
}

// --- SERVER LOAD PROCESS ---

void UServerSaveManager::RequestServerDownloadURL(const FString& Username)
{
    TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
    JsonObject->SetStringField(TEXT("username"), Username);

    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(JsonObject.ToSharedRef(), Writer);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	RequestUsernameMap.Add(Request, Username); // Map this request to the user
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::PlayerSaveAPI::ServerRequestDownloadURL);
	Request->OnProcessRequestComplete().BindUObject(this, &UServerSaveManager::OnRequestServerDownloadURLResponse);
	Request->SetURL(APIUrl);
    Request->SetVerb(TEXT("POST"));
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonPayload);
	Request->ProcessRequest();
}

void UServerSaveManager::OnRequestServerDownloadURLResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    const FString* UsernamePtr = RequestUsernameMap.Find(Request);
	if (!UsernamePtr) return;
	FString Username = *UsernamePtr;

	if (!bWasSuccessful || !Response.IsValid() || !EHttpResponseCodes::IsOk(Response->GetResponseCode()))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("RequestServerDownloadURL failed for user %s."), *Username);
		ActiveOperations.Remove(Username);
		RequestUsernameMap.Remove(Request);
		OnServerLoadFromS3Complete.Broadcast(false, nullptr, Username);
		return;
	}
	
	TSharedPtr<FJsonObject> OuterJsonObject;
	TSharedRef<TJsonReader<>> OuterReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(OuterReader, OuterJsonObject) && OuterJsonObject.IsValid())
	{
		FString BodyString;
		if (OuterJsonObject->TryGetStringField(TEXT("body"), BodyString))
		{
			TSharedPtr<FJsonObject> InnerJsonObject;
			TSharedRef<TJsonReader<>> InnerReader = TJsonReaderFactory<>::Create(BodyString);
			if (FJsonSerializer::Deserialize(InnerReader, InnerJsonObject) && InnerJsonObject.IsValid())
			{
				FString DownloadURL;
				if (InnerJsonObject->TryGetStringField(TEXT("downloadUrl"), DownloadURL))
				{
					RequestUsernameMap.Remove(Request); 
					TSharedRef<IHttpRequest> S3Request = DownloadDataFromS3(DownloadURL);
					RequestUsernameMap.Add(S3Request, Username); 
					return;
				}
			}
		}
	}
	UE_LOG(LogDedicatedServers, Error, TEXT("OnRequestServerDownloadURLResponse: Failed to parse response for user %s."), *Username);
    ActiveOperations.Remove(Username);
    RequestUsernameMap.Remove(Request);
    OnServerLoadFromS3Complete.Broadcast(false, nullptr, Username);
}

