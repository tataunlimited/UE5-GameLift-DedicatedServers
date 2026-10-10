#include "Game/DS_MatchGameMode.h"

#include "Game/DS_GameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSPlayerController.h"
#include "Player/DS_MatchPlayerState.h"
#include "Save/ServerSaveManager.h"
#include "UI/GameStats/GameStatsManager.h"

ADS_MatchGameMode::ADS_MatchGameMode()
{
	bUseSeamlessTravel = true;
	MatchStatus = EMatchStatus::WaitingForPlayers;
	PreMatchTimer.Type = ECountdownTimerType::PreMatch;
	MatchTimer.Type = ECountdownTimerType::Match;
	PostMatchTimer.Type = ECountdownTimerType::PostMatch;
}

void ADS_MatchGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (MatchStatus == EMatchStatus::WaitingForPlayers)
	{
		MatchStatus = EMatchStatus::PreMatch;
		StartCountdownTimer(PreMatchTimer);
	}
}

void ADS_MatchGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	RemovePlayerSession(Exiting);
	
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		this,
		&ADS_MatchGameMode::CheckIfAllPlayersLeft,
		15.0f,  // Short delay is enough
		false
	);

}

void ADS_MatchGameMode::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
	
	if (MatchStatus == EMatchStatus::WaitingForPlayers)
	{
		MatchStatus = EMatchStatus::PreMatch;
		StartCountdownTimer(PreMatchTimer);
	}
}

FString ADS_MatchGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId,
	const FString& Options, const FString& Portal)
{
	FString InitializedString = Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);

	const FString PlayerSessionId = UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));
	const FString Username = UGameplayStatics::ParseOption(Options, TEXT("Username"));

	if (ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(NewPlayerController); IsValid(DSPlayerController))
	{
		UE_LOG(LogTemp, Warning, TEXT("ADS_MatchGameMode::InitNewPlayer - Username: %s"), *Username);
		UE_LOG(LogTemp, Warning, TEXT("ADS_MatchGameMode::InitNewPlayer - DSPlayerController->Username: %s"), *DSPlayerController->Username);

		UE_LOG(LogTemp, Warning, TEXT("ADS_MatchGameMode::InitNewPlayer is valid"));
		DSPlayerController->PlayerSessionId = PlayerSessionId;
		DSPlayerController->Username = Username;
		DSPlayerController->PlayerState->SetPlayerName(Username);

		if (ADSPlayerState* DSPlayerState = Cast<ADSPlayerState>(DSPlayerController->PlayerState))
		{
			DSPlayerState->SetDisplayName(Username);
		}
	}
	
	return InitializedString;
}

void ADS_MatchGameMode::OnCountdownTimerFinished(ECountdownTimerType Type)
{
	Super::OnCountdownTimerFinished(Type);

	if (Type == ECountdownTimerType::PreMatch)
	{
		StopCountdownTimer(PreMatchTimer);
		MatchStatus = EMatchStatus::Match;
		StartCountdownTimer(MatchTimer);
		SetClientInputEnabled(true);
	}
	else if (Type == ECountdownTimerType::Match)
	{
		StopCountdownTimer(MatchTimer);
		MatchStatus = EMatchStatus::PostMatch;
		StartCountdownTimer(PostMatchTimer);
		SetClientInputEnabled(false);
		OnMatchEnded();
	}
	else if (Type == ECountdownTimerType::PostMatch)
	{
		StopCountdownTimer(PostMatchTimer);
		MatchStatus = EMatchStatus::SeamlessTravelling;
		TrySeamlessTravel(LobbyMap);
	}
}

void ADS_MatchGameMode::BeginPlay()
{
	Super::BeginPlay();
	GameStatsManager = NewObject<UGameStatsManager>(this, GameStatsManagerClass);
	GameStatsManager->OnUpdateLeaderboardSucceeded.AddDynamic(this, &ADS_MatchGameMode::OnLeaderboardUpdated);
	InitGameLift();
}

void ADS_MatchGameMode::EndMatchForPlayerStates()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Iterator->Get()); IsValid(DSPlayerController))
		{
			if (ADS_MatchPlayerState* MatchPlayerState = DSPlayerController->GetPlayerState<ADS_MatchPlayerState>(); IsValid(MatchPlayerState))
			{
				MatchPlayerState->OnMatchEnded(DSPlayerController->Username);
			}
		}
	}
}

void ADS_MatchGameMode::OnMatchEnded()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Iterator->Get()); IsValid(DSPlayerController))
		{
			if (ADSPlayerState* DSPlayerState = Cast<ADSPlayerState>(DSPlayerController->PlayerState))
			{
				UDS_GameInstanceSubsystem* GameInstanceSubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDS_GameInstanceSubsystem>();
				if (GameInstanceSubsystem && GameInstanceSubsystem->GetSaveManager())
				{
					UServerSaveManager* ServerSaveManager = GameInstanceSubsystem->GetSaveManager();
					ServerSaveManager->TriggerBlueprintSaveForPlayer(DSPlayerState->GetDisplayName(), DSPlayerController->GetPawn());
				}
			}
		}
	}
}

void ADS_MatchGameMode::UpdateLeaderboard(const TArray<FString>& LeaderboardNames)
{
	if (IsValid(GameStatsManager))
	{
		GameStatsManager->UpdateLeaderboard(LeaderboardNames);
	}
}

void ADS_MatchGameMode::OnLeaderboardUpdated()
{
	EndMatchForPlayerStates();
}

void ADS_MatchGameMode::CheckIfAllPlayersLeft()
{
	if (GetNumPlayers() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("All players have left. Returning to lobby..."));
		TrySeamlessTravel(LobbyMap);
	}
}



