#pragma once

#include "CoreMinimal.h"
#include "Save/SaveManagerBase.h"
#include "ServerSaveManager.generated.h"

// Delegate specifically for server-side load operations.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnServerLoadFromS3Complete, bool, bWasSuccessful, USaveGame*, LoadedPlayerData, FString, Username);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnServerSaveToS3Complete, bool, bWasSuccessful, const FString&, Username);

/**
 * @struct FServerAsyncOperationData
 * @brief Stores the state of an asynchronous save/load operation for a single user on the server.
 * This prevents data from different user requests from conflicting.
 */
USTRUCT()
struct FServerAsyncOperationData
{
	GENERATED_BODY()

	// The username of the player this operation is for.
	UPROPERTY()
	FString Username;

	// Serialized data for uploads.
	UPROPERTY()
	TArray<uint8> PlayerSerializedData;

	// S3 key received from the API, used for confirming an upload.
	UPROPERTY()
	FString PlayerS3Key;

	// A pointer to the HTTP request associated with this operation's API call (e.g., requesting a URL).
	// This allows us to map an HTTP response back to the user who initiated it.
	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> ContextRequest;
};

/**
 * @class UServerSaveManager
 * @brief Manages saving and loading player state from the server perspective.
 * This class is designed to handle requests for multiple users concurrently and safely.
 */
UCLASS()
class DEDICATEDSERVERS_API UServerSaveManager : public USaveManagerBase
{
	GENERATED_BODY()

public:
	// --- Public API for Server ---

	
	UFUNCTION(BlueprintCallable, Category = "Player Save Manager|S3|Server")
	void Server_SavePlayerStateToS3(USaveGame* PlayerSaveObject, const FString& Username);

	UFUNCTION(BlueprintCallable, Category = "Player Save Manager|S3|Server")
	void Server_LoadPlayerStateFromS3(const FString& Username);

	UFUNCTION(BlueprintCallable, Category = "Player Save Manager|S3|Server")
	void TriggerBlueprintSaveForPlayer(const FString& Username, APawn* Player);
	
	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Player Save Manager|Events")
	FOnServerLoadFromS3Complete OnServerLoadFromS3Complete;

	UPROPERTY(BlueprintAssignable, Category = "Player Save Manager|Events")
	FOnServerSaveToS3Complete OnServerSaveToS3Complete;
	
protected:
	// --- Overridden Handlers ---
	virtual void HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) override;
	virtual void HandleDownloadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Save Manager|S3|Server")
	void CreateAndSavePlayerData(const FString& Username, APawn* Player);
	
private:
	// --- Server Save Process ---
	void RequestServerUploadURL(const FString& Username);
	void OnRequestServerUploadURLResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void ConfirmServerS3Upload(const FString& Username);
	void OnConfirmServerS3UploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// --- Server Load Process ---
	void RequestServerDownloadURL(const FString& Username);
	void OnRequestServerDownloadURLResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// --- Multi-User State Management ---

	// Maps a username to their active asynchronous operation data.
	UPROPERTY()
	TMap<FString, FServerAsyncOperationData> ActiveOperations;
	
	// Maps an active HTTP request pointer to the username that initiated it. This is crucial for routing
	// asynchronous responses back to the correct user's operation data.
	TMap<FHttpRequestPtr, FString> RequestUsernameMap;
};
