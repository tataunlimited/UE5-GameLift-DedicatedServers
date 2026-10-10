#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "UI/HTTP/HTTPRequestManager.h"
#include "GameSessionsManager.generated.h"


/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UGameSessionsManager : public UHTTPRequestManager
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable)
	FAPIStatusMessage BroadcastJoinGameSessionMessage;

	void JoinGameSession(const FString& Action);
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> HomeAreaMap;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> PortalMap;

	UPROPERTY(EditDefaultsOnly)
	bool bIsAnyWhereFleet;

private:

	void FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	void CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
	FString GetUniquePlayerId() const;
	void HandleGameSessionStatus(const FString& Status, const FString& SessionId);
	void TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId);

	FTimerHandle CreateSessionTimer;

	UPROPERTY()
	FString CachedJoinAction;

};
