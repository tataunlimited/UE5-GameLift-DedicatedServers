#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerTravelComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleteSignature, bool, bWasSuccessful);

/**
 * This component is attached to the PlayerController and handles the RPC communication
 * for saving player data before traveling from a dedicated server.
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DEDICATEDSERVERS_API UPlayerTravelComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPlayerTravelComponent();

	/** Delegate that is broadcast on the client when the server confirms the save is complete. */
	UPROPERTY(BlueprintAssignable, Category = "Travel")
	FOnSaveCompleteSignature OnSaveComplete;

	/** [Client] Sends a request to the server to save this player's data. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestSavePlayerData();

protected:
	/** [Server] Callback for when the ServerSaveManager completes its save operation. */
	UFUNCTION()
	void OnServerSaveCompleted(bool bWasSuccessful, const FString& SavedUsername);

	/** [Server] Notifies the owning client that the save operation is complete. */
	UFUNCTION(Client, Reliable)
	void Client_OnSavePlayerDataComplete(bool bWasSuccessful);

private:
	// Store the username on the server to ensure the completion delegate is for the correct player.
	FString PlayerUsername;
};
