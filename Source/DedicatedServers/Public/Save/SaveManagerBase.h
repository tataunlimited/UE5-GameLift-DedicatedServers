#pragma once

#include "CoreMinimal.h"
#include "UI/HTTP/HTTPRequestManager.h"
#include "GameFramework/SaveGame.h"
#include "Interfaces/IHttpRequest.h"
#include "SaveManagerBase.generated.h"

// Forward-declare the TSharedPtr typedefs for HTTP requests.
// This is good practice and helps the compiler understand the types exist without full inclusion.
//using FHttpRequestPtr = TSharedPtr<IHttpRequest, ESPMode::ThreadSafe>;
//using FHttpResponsePtr = TSharedPtr<IHttpResponse, ESPMode::ThreadSafe>;


// Delegate for broadcasting save completion status, common to both client and server.

UCLASS(Abstract)
class DEDICATEDSERVERS_API USaveManagerBase : public UHTTPRequestManager
{
	GENERATED_BODY()

public:

	// The class to use when deserializing player save data.
	UPROPERTY(EditDefaultsOnly, Category="Player Save Manager|Configuration")
	TSubclassOf<USaveGame> PlayerSaveGameClass;

	UFUNCTION(BlueprintImplementableEvent)
	void Initialize();

protected:
	TSharedRef<IHttpRequest> UploadDataToS3(const FString& UploadURL, const TArray<uint8>& SerializedData);
	TSharedRef<IHttpRequest> DownloadDataFromS3(const FString& DownloadURL);
	virtual void HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	virtual void HandleDownloadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	
private:

	void OnUploadToS3ResponseInternal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void OnDownloadFromS3ResponseInternal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
};

