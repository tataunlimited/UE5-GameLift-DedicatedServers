#pragma once

#include "CoreMinimal.h"
#include "LobbyPlayerInfo.h"
#include "GameFramework/Info.h"
#include "LobbyState.generated.h"

USTRUCT()
struct FLobbyPlayerInfoDelta
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLobbyPlayerInfo> AddedPlayers{};

	UPROPERTY()
	TArray<FLobbyPlayerInfo> RemovedPlayers{};

	UPROPERTY()
	TArray<FLobbyPlayerInfo> UpdatedPlayers;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerInfoChanged, const FLobbyPlayerInfo&, PlayerInfo);

UCLASS()
class DEDICATEDSERVERS_API ALobbyState : public AInfo
{
	GENERATED_BODY()

public:
	ALobbyState();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerInfoChanged OnPlayerInfoAdded;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerInfoChanged OnPlayerInfoRemoved;

	UPROPERTY(BlueprintAssignable)
	FOnPlayerInfoChanged OnPlayerInfoUpdated;

	void AddPlayerInfo(const FLobbyPlayerInfo& PlayerInfo);
	void RemovePlayerInfo(const FString& Username);
	TArray<FLobbyPlayerInfo> GetPlayers() const;

	void SetPlayerReadyState(const FString& Username, bool bIsReady);
	void SetPlayerVoiceStatus(const FString& Username, bool bVoiceConnected, bool bIsSpeaking);

protected:

	UFUNCTION()
	void OnRep_LobbyPlayerInfo();

private:

	UPROPERTY(ReplicatedUsing = OnRep_LobbyPlayerInfo)
	FLobbyPlayerInfoArray PlayerInfoArray;

	UPROPERTY()
	FLobbyPlayerInfoArray LastPlayerInfoArray;

	FLobbyPlayerInfoDelta ComputePlayerInfoDelta(const TArray<FLobbyPlayerInfo>& OldArray, const TArray<FLobbyPlayerInfo>& NewArray);

};
