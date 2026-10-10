#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UI/HTTP/HTTPRequestManager.h"
#include "APITestManager.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnListFleetssResponeReceived, const FDSListFleetsResponse&, ListFleetsResponse, bool, bWasSuccessful);
class UListFleetsBox;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UAPITestManager : public UHTTPRequestManager
{
	GENERATED_BODY()

public:

	UFUNCTION()
	void ListFleets();

	UPROPERTY()
	FOnListFleetssResponeReceived OnListFleetsResponseReceived;
	
	void ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSucceeded);

	
};
