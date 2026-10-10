#include "UI/Lobby/LobbyPlayerBox.h"

#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "DedicatedServers/DedicatedServers.h"
#include "Game/DSGameState.h"
#include "Lobby/LobbyState.h"
#include "Lobby/LobbyPlayerInfo.h"
#include "Player/DSPlayerController.h"
#include "UI/Lobby/PlayerLabel.h"

void ULobbyPlayerBox::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (IsValid(Button_Ready))
	{
		Button_Ready->OnClicked.AddDynamic(this, &ULobbyPlayerBox::OnReadyClicked);
	}
	if (IsValid(Button_Refresh))
	{
		Button_Refresh->OnClicked.AddDynamic(this, &ULobbyPlayerBox::OnRefreshButtonClicked);
	}

	SetupPlayerBox();
}
void ULobbyPlayerBox::SetupPlayerBox()
{
	ADSGameState* DSGameState = GetWorld()->GetGameState<ADSGameState>();
	if (!IsValid(DSGameState))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("ULobbyPlayerBox::NativeOnInitialized: GameState is invalid"));
		return;
	}

	if (IsValid(DSGameState->LobbyState))
	{
		OnLobbyStateInitialized(DSGameState->LobbyState);
	}
	else
	{
		DSGameState->OnLobbyStateInitialized.AddDynamic(this, &ULobbyPlayerBox::OnLobbyStateInitialized);
	}
}

void ULobbyPlayerBox::OnRefreshButtonClicked()
{
	if (!bIsInitialized)
	{
		SetupPlayerBox();
	}

	ADSGameState* DSGameState = GetWorld()->GetGameState<ADSGameState>();
	if (!IsValid(DSGameState))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("ULobbyPlayerBox::OnRefreshButtonClicked: GameState is invalid"));
		return;
	}

	if (IsValid(DSGameState->LobbyState))
	{
		UpdatePlayerInfo(DSGameState->LobbyState);
	}
	UE_LOG(LogDedicatedServers, Warning, TEXT("OnRefreshButtonClicked::UpdatePlayerInfo"));

}

void ULobbyPlayerBox::UpdatePlayerInfo(ALobbyState* LobbyState)
{
	UE_LOG(LogDedicatedServers, Warning, TEXT("ULobbyPlayerBox::UpdatePlayerInfo"));

	ScrollBox_PlayerInfo->ClearChildren();
	for (const FLobbyPlayerInfo& PlayerInfo : LobbyState->GetPlayers())
	{
		CreateAndAddPlayerLabel(PlayerInfo);
	}
}

void ULobbyPlayerBox::OnLobbyStateInitialized(ALobbyState* LobbyState)
{
	if (!IsValid(LobbyState))
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("ULobbyPlayerBox::NativeOnInitialized: LobbyState is invalid"));
		return;
	}

	UE_LOG(LogDedicatedServers, Warning, TEXT("ULobbyPlayerBox::OnLobbyStateInitialized"));
	LobbyState->OnPlayerInfoAdded.AddDynamic(this, &ULobbyPlayerBox::CreateAndAddPlayerLabel);
	LobbyState->OnPlayerInfoRemoved.AddDynamic(this, &ULobbyPlayerBox::OnPlayerRemoved);
	LobbyState->OnPlayerInfoUpdated.AddDynamic(this, &ULobbyPlayerBox::OnPlayerReadyStatusChanged);
	
	UpdatePlayerInfo(LobbyState);
	bIsInitialized = true;
	
}

void ULobbyPlayerBox::CreateAndAddPlayerLabel(const FLobbyPlayerInfo& PlayerInfo)
{

	if (FindPlayerLabel(PlayerInfo.Username)) return;
	
	UPlayerLabel* PlayerLabel = CreateWidget<UPlayerLabel>(this, PlayerLabelClass);
	if (!IsValid(PlayerLabel)) return;

	PlayerLabel->SetUsername(PlayerInfo.Username);
	PlayerLabel->SetReadyStatus(PlayerInfo.bIsReady);
	PlayerLabel->SetVoiceStatus(PlayerInfo.bVoiceConnected, PlayerInfo.bIsSpeaking);

	ScrollBox_PlayerInfo->AddChild(PlayerLabel);

}

void ULobbyPlayerBox::OnPlayerRemoved(const FLobbyPlayerInfo& PlayerInfo)
{
	if (UPlayerLabel* PlayerLabel = FindPlayerLabel(PlayerInfo.Username))
	{
		ScrollBox_PlayerInfo->RemoveChild(PlayerLabel);
	}
}

void ULobbyPlayerBox::OnPlayerReadyStatusChanged(const FLobbyPlayerInfo& PlayerInfo)
{
	UE_LOG(LogTemp, Warning, TEXT("OnPlayerReadyStatusChanged called for %s (Ready: %d)"), *PlayerInfo.Username, PlayerInfo.bIsReady);

	if (UPlayerLabel* Label = FindPlayerLabel(PlayerInfo.Username))
	{
		Label->SetReadyStatus(PlayerInfo.bIsReady);
		Label->SetVoiceStatus(PlayerInfo.bVoiceConnected, PlayerInfo.bIsSpeaking);
	}
}

UPlayerLabel* ULobbyPlayerBox::FindPlayerLabel(const FString& Username)
{
	for (UWidget* Child: ScrollBox_PlayerInfo->GetAllChildren())
	{
		UPlayerLabel* PlayerLabel = Cast<UPlayerLabel>(Child);
		if (IsValid(PlayerLabel) && PlayerLabel->GetUsername() == Username)
		{
			return PlayerLabel;
		}
	}
	return nullptr;
}

void ULobbyPlayerBox::OnReadyClicked()
{
	if (ADSPlayerController* PC = Cast<ADSPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->Server_SetReady(true);
		Button_Ready->SetIsEnabled(false);
	}
}


