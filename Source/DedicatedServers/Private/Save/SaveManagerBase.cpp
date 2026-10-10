#include "Save/SaveManagerBase.h"
#include "HttpModule.h"

TSharedRef<IHttpRequest> USaveManagerBase::UploadDataToS3(const FString& UploadURL, const TArray<uint8>& SerializedData)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &USaveManagerBase::OnUploadToS3ResponseInternal);
	Request->SetURL(UploadURL);
	Request->SetVerb(TEXT("PUT"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/octet-stream"));
	Request->SetContent(SerializedData);
	Request->ProcessRequest();
	return Request;
}

TSharedRef<IHttpRequest> USaveManagerBase::DownloadDataFromS3(const FString& DownloadURL)
{
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &USaveManagerBase::OnDownloadFromS3ResponseInternal);
	Request->SetURL(DownloadURL);
	Request->SetVerb(TEXT("GET"));
	Request->ProcessRequest();
	return Request;
}

void USaveManagerBase::OnUploadToS3ResponseInternal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Defer logic to the appropriate derived class
	HandleUploadResponse(Request, Response, bWasSuccessful);
}

void USaveManagerBase::OnDownloadFromS3ResponseInternal(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Defer logic to the appropriate derived class
	HandleDownloadResponse(Request, Response, bWasSuccessful);
}

void USaveManagerBase::HandleUploadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Base implementation. Derived classes should override this.
	UE_LOG(LogTemp, Warning, TEXT("USaveManagerBase::HandleUploadResponse should be overridden in a child class."));
}

void USaveManagerBase::HandleDownloadResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	// Base implementation. Derived classes should override this.
	UE_LOG(LogTemp, Warning, TEXT("USaveManagerBase::HandleDownloadResponse should be overridden in a child class."));
}
