#include "Player/DSPlayerController.h"

#include "Algo/AllOf.h"
#include "Game/DSGameState.h"
#include "Game/DS_LobbyGameMode.h"
#include "Lobby/LobbyState.h"

ADSPlayerController::ADSPlayerController()
{
	SingleTripTime = 0.0f;
	Username = "";
	PlayerSessionId = "";
}

void ADSPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if (GetNetMode() == NM_Standalone) return;
	if (IsLocalPlayerController())
	{
		Server_Ping(GetWorld()->GetTimeSeconds());
	}
}

void ADSPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
}

void ADSPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	if (IsLocalController())
	{
		Server_Ping(GetWorld()->GetTimeSeconds());
	}
}

void ADSPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void ADSPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// Only handle Discord disconnection on client side
	// if (IsLocalPlayerController() && EndPlayReason == EEndPlayReason::Destroyed)
	// {
	// 	if (UGameInstance* GameInstance = GetGameInstance())
	// 	{
	// 		if (UDiscordManager* DiscordManager = GameInstance->GetSubsystem<UDiscordManager>())
	// 		{
	// 			UE_LOG(LogTemp, Log, TEXT("ADSPlayerController::EndPlay - Disconnecting from Discord"));
	// 			DiscordManager->DisconnectFromDiscord();
	// 		}
	// 	}
	// }
}

void ADSPlayerController::PawnLeavingGame()
{
	OnBeforePawnLeavingGame();
	Super::PawnLeavingGame();
}


void ADSPlayerController::Server_SetReady_Implementation(bool bIsReady)
{
	if (ADSGameState* GS = GetWorld()->GetGameState<ADSGameState>())
	{
		if (ALobbyState* LobbyState = GS->LobbyState)
		{
			LobbyState->SetPlayerReadyState(Username, bIsReady);

			const TArray<FLobbyPlayerInfo>& Players = LobbyState->GetPlayers();

			const bool bAllReady = Players.Num() > 0 && Algo::AllOf(Players, [](const FLobbyPlayerInfo& Info)
			{
				return Info.bIsReady;
			});
			
			if (bAllReady)
			{
				AGameModeBase* BaseGM = GetWorld()->GetAuthGameMode();
				if (ADS_LobbyGameMode* GM = Cast<ADS_LobbyGameMode>(BaseGM))
				{
					GM->StartMatchManually();
				}
			}
		}
	}
}

void ADSPlayerController::Server_SetVoiceStatus_Implementation(bool bVoiceConnected, bool bIsSpeaking)
{
	if (ADSGameState* GS = GetWorld()->GetGameState<ADSGameState>())
	{
		if (ALobbyState* LobbyState = GS->LobbyState)
		{
			LobbyState->SetPlayerVoiceStatus(Username, bVoiceConnected, bIsSpeaking);
		}
	}
}

void ADSPlayerController::Client_SetInputEnabled_Implementation(bool bEnabled)
{
	if (bEnabled)
		EnableInput(this);
	else
		DisableInput(this);
}

void ADSPlayerController::Client_TimerStopped_Implementation(float CountDownTimeLeft, ECountdownTimerType Type) const
{
	OnTimerStopped.Broadcast(CountDownTimeLeft - SingleTripTime, Type);
}

void ADSPlayerController::Client_TimerUpdated_Implementation(float CountDownTimeLeft, ECountdownTimerType Type) const
{
	OnTimerUpdated.Broadcast(CountDownTimeLeft - SingleTripTime, Type);
}

void ADSPlayerController::Server_Ping_Implementation(float TimeOfRequest)
{
	Client_Pong(TimeOfRequest);
}

void ADSPlayerController::Client_Pong_Implementation(float TimeOfRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfRequest;
	SingleTripTime = RoundTripTime * 0.5f;
}

