#pragma once

#include "CoreMinimal.h"
#include "Save/SaveManagerBase.h"
#include "ClientSaveManager.generated.h"

// Delegate specifically for client-side load operations, which include HomeArea data.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLoadFromS3Complete, bool, bWasSuccessful, USaveGame*, LoadedHomeAreaData, USaveGame*, LoadedPlayerData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveToS3Complete, bool, bWasSuccessfu);

class UObject;
class USaveGame;

/**
 * @class UClientSaveManager
 * @brief Manages saving and loading player state from the client perspective.
 * Handles both Player and HomeArea save data, orchestrating parallel S3 operations and API confirmations.
 */
UCLASS()
class DEDICATEDSERVERS_API UClientSaveManager : public USaveManagerBase
{
	GENERATED_BODY()

public:
	// --- Public API for Client ---

	UFUNCTION(BlueprintCallable, Category = "Player Save Manager|S3|Client")
	void SavePlayerStateToS3(USaveGame* PlayerSaveObject, USaveGame* HomeAreaSaveObject);

	UFUNCTION(BlueprintCallable, Category = "Player Save Manager|S3|Client")
	void LoadPlayerStateFromS3();

	UFUNCTION(BlueprintCallable, Category = "Player Save Manager|S3|Client")
	void TriggerBlueprintClientSave(AGameModeBase* GameMode);

	// --- Delegates ---

	UPROPERTY(BlueprintAssignable, Category = "Player Save Manager|Events")
	FOnLoadFromS3Complete OnLoadFromS3Complete;

	// --- Configuration ---
    
	UPROPERTY(EditDefaultsOnly, Category="Player Save Manager|Configuration")
	TSubclassOf<USaveGame> HomeAreaSaveGameClass;

	UFUNCTION(BlueprintPure, Category = "Player Save Manage")
	FString GetUsername() const;

	UPROPERTY(BlueprintAssignable, Category = "Player Save Manager|Events")
	FOnSaveToS3Complete OnSaveToS3Complete;

protected:
	// --- Overridden Handlers ---
	virtual void HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) override;
	virtual void HandleDownloadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Player Save Manager|S3|Client")
	void CreateAndSaveLocalData(AGameModeBase* GameMode);

private:
	// --- Client Save Process ---
	void RequestClientUploadURLs();
	void OnRequestClientUploadURLsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void ConfirmClientS3Upload();
	void OnConfirmClientS3UploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// --- Client Load Process ---
	void RequestClientDownloadURLs();
	void OnRequestClientDownloadURLsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	// --- Temporary Data Storage for Client's Chained Requests ---
	TArray<uint8> TempPlayerSerializedData;
	TArray<uint8> TempHomeAreaSerializedData;
	FString TempPlayerS3Key;
	FString TempHomeAreaS3Key;
	USaveGame* TempLoadedPlayerSave = nullptr;
	USaveGame* TempLoadedHomeAreaSave = nullptr;
	int32 PendingDownloads = 0;
	int32 PendingUploads = 0;
};
