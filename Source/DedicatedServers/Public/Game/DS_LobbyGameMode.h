#pragma once

#include "CoreMinimal.h"
#include "DS_GameModeBase.h"
#include "GameLiftServerSDK.h"
#include "DS_LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADS_LobbyGameMode : public ADS_GameModeBase
{
	GENERATED_BODY()
public:
	ADS_LobbyGameMode();
	void CheckAndStartLobbyCountdown();
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	UFUNCTION()
	void StartMatchManually();
	
protected:
	UFUNCTION()
	virtual void BeginPlay() override;
	virtual void OnCountdownTimerFinished(ECountdownTimerType Type) override;
	void CheckAndStopLobbyCountdown();
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = L"") override;
	virtual void InitSeamlessTravelPlayer(AController* NewController) override;

	UPROPERTY()
	ELobbyStatus LobbyStatus;

	UPROPERTY(EditDefaultsOnly)
	int32 MinPlayers;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DestinationMap;

private:

	UPROPERTY(EditDefaultsOnly)
	FCountdownTimerHandle LobbyCountdownTimer;

	void TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage);
	void AddPlayerInfoToLobbyState(AController* Player) const;
	void RemovePlayerInfoFromLobbyState(AController* Player) const;
	void TryAddPlayerInfoWithRetries(AController* Player, int32 Attempt);
	
};





