#include "Game/DS_LobbyGameMode.h"
#include "Game/DSGameState.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Lobby/LobbyState.h"
#include "Player/DSPlayerController.h"
#include "Player/DSPlayerState.h"

ADS_LobbyGameMode::ADS_LobbyGameMode()
{
	bUseSeamlessTravel = true;
	LobbyStatus = ELobbyStatus::WaitingForPlayers;
	MinPlayers = 1;
	LobbyCountdownTimer.Type = ECountdownTimerType::LobbyCountdown;
}

void ADS_LobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}



void ADS_LobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	CheckAndStopLobbyCountdown();
	RemovePlayerSession(Exiting);
	if (LobbyStatus != ELobbyStatus::SeamlessTravelling)
	{
		RemovePlayerInfoFromLobbyState(Exiting);
	}
}

void ADS_LobbyGameMode::StartMatchManually()
{
	LobbyStatus = ELobbyStatus::CountdownToSeamlessTravel;
	StartCountdownTimer(LobbyCountdownTimer);
}

void ADS_LobbyGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId,
                                 FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

	TryAcceptPlayerSession(PlayerSessionId, Username, ErrorMessage);
}

FString ADS_LobbyGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

	if (ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(NewPlayerController); IsValid(DSPlayerController))
	{
		DSPlayerController->PlayerSessionId = PlayerSessionId;
		DSPlayerController->Username = Username;
		DSPlayerController->PlayerState->SetPlayerName(Username);
		if (ADSPlayerState* DSPlayerState = Cast<ADSPlayerState>(DSPlayerController->PlayerState))
		{
			DSPlayerState->SetDisplayName(Username);
		}
	}

	if (LobbyStatus != ELobbyStatus::SeamlessTravelling)
	{
		AddPlayerInfoToLobbyState(NewPlayerController);
	}
	
	return InitializedString;
}

void ADS_LobbyGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
	TryAddPlayerInfoWithRetries(NewController, 0);
}

void ADS_LobbyGameMode::TryAcceptPlayerSession(const FString& PlayerSessionId, const FString& Username, FString& OutErrorMessage)
{
	 if (PlayerSessionId.IsEmpty() || Username.IsEmpty())
	 {
	 	OutErrorMessage = "PlayerSessionId and/or Username is invalid";
	 	return;
	 }

#if WITH_GAMELIFT
	Aws::GameLift::Server::Model::DescribePlayerSessionsRequest DescribePlayerSessionRequest;
	DescribePlayerSessionRequest.SetPlayerSessionId(TCHAR_TO_ANSI(*PlayerSessionId));
	
	const auto& DescribePlayerSessionsOutcome = Aws::GameLift::Server::DescribePlayerSessions(DescribePlayerSessionRequest);
	if (!DescribePlayerSessionsOutcome.IsSuccess())
	{
		OutErrorMessage = TEXT("DescribePlayerSessions failed.");
	}

	const auto& DescribePlayerSessionsResult = DescribePlayerSessionsOutcome.GetResult();
	int32 Count = 0;
	const Aws::GameLift::Server::Model::PlayerSession* PlayerSessions = DescribePlayerSessionsResult.GetPlayerSessions(Count);
	if(PlayerSessions == nullptr || Count == 0)
	{
		OutErrorMessage = TEXT("GetPlayerSession failed.");
		return;
	}

	for (int32 i = 0; i < Count; i++)
	{
		const Aws::GameLift::Server::Model::PlayerSession& PlayerSession = PlayerSessions[i];
		if (!Username.Equals(PlayerSession.GetPlayerId())) continue;
		if (PlayerSession.GetStatus() != Aws::GameLift::Server::Model::PlayerSessionStatus::RESERVED)
		{
			OutErrorMessage = FString::Printf(TEXT("Session for %s not RESERVED; Fail PreLogin."), *Username);
			continue;
		}

		const auto& AcceptPlayerSessionOutcome = Aws::GameLift::Server::AcceptPlayerSession(TCHAR_TO_ANSI(*PlayerSessionId));
		OutErrorMessage = AcceptPlayerSessionOutcome.IsSuccess() ? "" : FString::Printf(TEXT("Failed to accept player session for %s."), *Username);
		if (AcceptPlayerSessionOutcome.IsSuccess())
			return;
	}
#endif
}
void ADS_LobbyGameMode::TryAddPlayerInfoWithRetries(AController* Player, int32 Attempt)
{
	const int32 MaxAttempts = 5;
	const float RetryDelay = 0.2f;

	ADSGameState* GS = GetGameState<ADSGameState>();
	if (IsValid(GS) && IsValid(GS->LobbyState))
	{
		AddPlayerInfoToLobbyState(Player);
		UE_LOG(LogTemp, Warning, TEXT("Player info added successfully on attempt %d"), Attempt);
	}
	else if (Attempt < MaxAttempts)
	{
		UE_LOG(LogTemp, Warning, TEXT("LobbyState not ready, retrying attempt %d..."), Attempt + 1);

		FTimerHandle TimerHandle;
		GetWorldTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateUObject(
			this, &ADS_LobbyGameMode::TryAddPlayerInfoWithRetries, Player, Attempt + 1), RetryDelay, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to add player info after %d attempts."), MaxAttempts);
	}
}

void ADS_LobbyGameMode::AddPlayerInfoToLobbyState(AController* Player) const
{
	
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Player);
	ADSGameState* DSGameState = GetGameState<ADSGameState>();
	
	if (IsValid(DSGameState) && IsValid(DSGameState->LobbyState) && IsValid(DSPlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("ADS_LobbyGameMode::AddPlayerInfoToLobbyState - Username: %s"), *DSPlayerController->Username);
		if (DSPlayerController->Username.IsEmpty())
		{
			if (ADSPlayerState* DSPlayerState = Cast<ADSPlayerState>(DSPlayerController->PlayerState))
			{
				DSPlayerController->Username = DSPlayerState->GetDisplayName();
			}
		}

		FLobbyPlayerInfo PlayerInfo(DSPlayerController->Username);
		DSGameState->LobbyState->AddPlayerInfo(PlayerInfo);
	}
}

void ADS_LobbyGameMode::RemovePlayerInfoFromLobbyState(AController* Player) const
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Player);
	ADSGameState* DSGameState = GetGameState<ADSGameState>();
	if (IsValid(DSGameState) && IsValid(DSGameState->LobbyState) && IsValid(DSPlayerController))
	{
		DSGameState->LobbyState->RemovePlayerInfo(DSPlayerController->Username);
	}
}


void ADS_LobbyGameMode::CheckAndStartLobbyCountdown()
{
	
	if (GetNumPlayers() >= MinPlayers && LobbyStatus == ELobbyStatus::WaitingForPlayers)
	{
		LobbyStatus = ELobbyStatus::CountdownToSeamlessTravel;
		StartCountdownTimer(LobbyCountdownTimer);
	}
}

void ADS_LobbyGameMode::CheckAndStopLobbyCountdown()
{
	if (GetNumPlayers() -1 < 1 /*MinPlayers*/ && LobbyStatus == ELobbyStatus::CountdownToSeamlessTravel)
	{
		LobbyStatus = ELobbyStatus::WaitingForPlayers;
		StopCountdownTimer(LobbyCountdownTimer);
	}
}

void ADS_LobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	InitGameLift();
}

void ADS_LobbyGameMode::OnCountdownTimerFinished(ECountdownTimerType Type)
{
	Super::OnCountdownTimerFinished(Type);

	if (Type == ECountdownTimerType::LobbyCountdown)
	{
		StopCountdownTimer(LobbyCountdownTimer);
		LobbyStatus = ELobbyStatus::SeamlessTravelling;
		TrySeamlessTravel(DestinationMap);
	}
}


