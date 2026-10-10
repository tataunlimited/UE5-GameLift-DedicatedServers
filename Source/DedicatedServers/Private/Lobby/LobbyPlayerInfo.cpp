#include "Lobby/LobbyPlayerInfo.h"

void FLobbyPlayerInfoArray::SetPlayerReadyState(const FString& Username, bool bReady)
{
	UE_LOG(LogTemp, Warning, TEXT("SetPlayerReadyState: Looking for %s"), *Username);

	for (FLobbyPlayerInfo& Player : Players)
	{
		if (Player.Username == Username)
		{
			UE_LOG(LogTemp, Warning, TEXT("Found %s, setting ready to %d"), *Username, bReady);
			Player.bIsReady = bReady;
			MarkItemDirty(Player);
			break;
		}
	}
}

void FLobbyPlayerInfoArray::AddPlayer(const FLobbyPlayerInfo& NewPlayerInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("Adding player %s"), *NewPlayerInfo.Username);
	int32 Index = Players.Add(NewPlayerInfo);
	MarkItemDirty(Players[Index]);
	Players[Index].PostReplicatedAdd(*this);
}

void FLobbyPlayerInfoArray::RemovePlayer(const FString& Username)
{
	UE_LOG(LogTemp, Warning, TEXT("Removing player %s"), *Username);
	for (int32 PlayerIndex = 0; PlayerIndex < Players.Num(); ++PlayerIndex)
	{
		FLobbyPlayerInfo& PlayerInfo = Players[PlayerIndex];
		if (PlayerInfo.Username == Username)
		{
			PlayerInfo.PreReplicatedRemove(*this);
			Players.RemoveAtSwap(PlayerIndex);
			MarkArrayDirty();
			break;
		}
	}
}

void FLobbyPlayerInfoArray::SetPlayerVoiceStatus(const FString& Username, bool bVoiceConnected, bool bIsSpeaking)
{
	for (FLobbyPlayerInfo& Player : Players)
	{
		if (Player.Username == Username)
		{
			Player.bVoiceConnected = bVoiceConnected;
			Player.bIsSpeaking = bIsSpeaking;
			MarkItemDirty(Player);
			break;
		}
	}
}
