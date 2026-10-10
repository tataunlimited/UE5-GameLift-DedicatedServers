#include "Lobby/LobbyState.h"
#include "Net/UnrealNetwork.h"


ALobbyState::ALobbyState()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ALobbyState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALobbyState, PlayerInfoArray);
}

void ALobbyState::AddPlayerInfo(const FLobbyPlayerInfo& PlayerInfo)
{
	PlayerInfoArray.AddPlayer(PlayerInfo);
}

void ALobbyState::RemovePlayerInfo(const FString& Username)
{
	PlayerInfoArray.RemovePlayer(Username);
}

TArray<FLobbyPlayerInfo> ALobbyState::GetPlayers() const
{
	return PlayerInfoArray.Players;
}

void ALobbyState::SetPlayerReadyState(const FString& Username, bool bIsReady)
{
	PlayerInfoArray.SetPlayerReadyState(Username, bIsReady);
}

void ALobbyState::SetPlayerVoiceStatus(const FString& Username, bool bVoiceConnected, bool bIsSpeaking)
{
	PlayerInfoArray.SetPlayerVoiceStatus(Username, bVoiceConnected, bIsSpeaking);
}

void ALobbyState::OnRep_LobbyPlayerInfo()
{
	FLobbyPlayerInfoDelta Delta = ComputePlayerInfoDelta(LastPlayerInfoArray.Players, PlayerInfoArray.Players);
	for (const auto& PlayerInfo : Delta.AddedPlayers)
	{
		OnPlayerInfoAdded.Broadcast(PlayerInfo);
	}
	for (const auto& PlayerInfo : Delta.RemovedPlayers)
	{
		OnPlayerInfoRemoved.Broadcast(PlayerInfo);
	}
	for (const auto& PlayerInfo : Delta.UpdatedPlayers)
	{
		OnPlayerInfoUpdated.Broadcast(PlayerInfo);
	}
	
	LastPlayerInfoArray = PlayerInfoArray;
}

FLobbyPlayerInfoDelta ALobbyState::ComputePlayerInfoDelta(const TArray<FLobbyPlayerInfo>& OldArray, const TArray<FLobbyPlayerInfo>& NewArray)
{
	FLobbyPlayerInfoDelta Delta;

	TMap<FString, const FLobbyPlayerInfo*> OldMap;
	TMap<FString, const FLobbyPlayerInfo*> NewMap;
	

	for (const FLobbyPlayerInfo& PlayerInfo : OldArray)
	{
		OldMap.Add(PlayerInfo.Username, &PlayerInfo);
	}
	for (const FLobbyPlayerInfo& PlayerInfo : NewArray)
	{
		NewMap.Add(PlayerInfo.Username, &PlayerInfo);
	}

	for (const auto& OldPlayerInfo : OldArray)
	{
		if (!NewMap.Contains(OldPlayerInfo.Username))
		{
			Delta.RemovedPlayers.Add(OldPlayerInfo);
		}
	}

	for (const auto& NewPlayerInfo : NewArray)
	{
		if (!OldMap.Contains(NewPlayerInfo.Username))
		{
			Delta.AddedPlayers.Add(NewPlayerInfo);
		}
		else
		{

			const FLobbyPlayerInfo* OldInfo = OldMap[NewPlayerInfo.Username];
			if (OldInfo->bIsReady != NewPlayerInfo.bIsReady || OldInfo->bIsSpeaking != NewPlayerInfo.bIsSpeaking || OldInfo->bVoiceConnected != NewPlayerInfo.bVoiceConnected)
			{
				Delta.UpdatedPlayers.Add(NewPlayerInfo);
			}
		}
	}

	return Delta;
}


