#include "Game/DS_GameModeBase.h"
#include "GameLiftServerSDK.h"
#include "Game/DS_GameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSPlayerController.h"
#include "DedicatedServers/DedicatedServers.h"


void ADS_GameModeBase::SpawnSelectedCharacter(APlayerController* PlayerController, TSubclassOf<APawn> PawnClassToSpawn)
{
	if (PlayerController == nullptr || PawnClassToSpawn == nullptr)
	{
		return;
	}

	// 1. Get a spawn location using FindPlayerStart and get its transform.
	AActor* StartSpot = FindPlayerStart(PlayerController);
	FVector SpawnLocation = StartSpot->GetActorLocation();
	FRotator SpawnRotation = StartSpot->GetActorRotation();

	// Set up spawn parameters. The "Owner" is needed for things like RPCs from the Pawn.
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = PlayerController;
	SpawnParams.Instigator = PlayerController->GetInstigator();

	// 2. Use SpawnActor to create an instance of the chosen character Pawn.
	APawn* NewPawn = GetWorld()->SpawnActor<APawn>(PawnClassToSpawn, SpawnLocation, SpawnRotation, SpawnParams);
    
	// If the pawn was spawned successfully...
	if (NewPawn)
	{
		// 3. ...use Possess to give the player control over their new Pawn. ✨
		PlayerController->Possess(NewPawn);
	}
}

void ADS_GameModeBase::StartCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle)
{
	CountdownTimerHandle.TimerFinishedDelegate.BindWeakLambda(this, [&]()
	{
		OnCountdownTimerFinished(CountdownTimerHandle.Type);
	});

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle.TimerFinishedHandle,
		CountdownTimerHandle.TimerFinishedDelegate,
		CountdownTimerHandle.CountdownTime,
		false
	);

	CountdownTimerHandle.TimerUpdateDelegate.BindWeakLambda(this, [&]()
	{
		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			UpdateCountdownTimer(CountdownTimerHandle);
		}
	});

	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle.TimerUpdateHandle,
		CountdownTimerHandle.TimerUpdateDelegate,
		CountdownTimerHandle.CountdownUpdateInterval,
		true
	);
	UpdateCountdownTimer(CountdownTimerHandle);
}

void ADS_GameModeBase::StopCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle)
{
	CountdownTimerHandle.State = ECountdownTimerState::Stopped;
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle.TimerFinishedHandle);
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle.TimerUpdateHandle);
	if (CountdownTimerHandle.TimerFinishedDelegate.IsBound())
	{
		CountdownTimerHandle.TimerFinishedDelegate.Unbind();
	}
	if (CountdownTimerHandle.TimerUpdateDelegate.IsBound())
	{
		CountdownTimerHandle.TimerUpdateDelegate.Unbind();
	}

	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Iterator->Get());
		if (IsValid(DSPlayerController))
		{
			DSPlayerController->Client_TimerStopped(0.f, CountdownTimerHandle.Type);
		}
	}
	
}

void ADS_GameModeBase::UpdateCountdownTimer(const FCountdownTimerHandle& CountdownTimerHandle)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Iterator->Get());
		if (IsValid(DSPlayerController))
		{
			const float CountdownTimeLeft = CountdownTimerHandle.CountdownTime - GetWorldTimerManager().GetTimerElapsed(CountdownTimerHandle.TimerFinishedHandle);
			DSPlayerController->Client_TimerUpdated(CountdownTimeLeft, CountdownTimerHandle.Type);
		}
	}
}

void ADS_GameModeBase::OnCountdownTimerFinished(ECountdownTimerType Type)
{
}

void ADS_GameModeBase::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
	OnSeamlessTravel(NewController);
}

void ADS_GameModeBase::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	OnPostSeamlessTravel();
}

void ADS_GameModeBase::TrySeamlessTravel(TSoftObjectPtr<UWorld> DestinationMap)
{
	const FString MapName = DestinationMap.ToSoftObjectPath().GetAssetName();
	if (GIsEditor)
	{
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationMap);
	}
	else
	{
		GetWorld()->ServerTravel(MapName);
	}
}

void ADS_GameModeBase::RemovePlayerSession(AController* Exiting)
{
	ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Exiting);
	if (!IsValid(DSPlayerController)) return;

#if WITH_GAMELIFT
	const FString PlayerSessionId = DSPlayerController->PlayerSessionId;
	if (!PlayerSessionId.IsEmpty())
	{
		Aws::GameLift::Server::RemovePlayerSession(TCHAR_TO_ANSI(*PlayerSessionId));
		UE_LOG(LogTemp, Warning, TEXT("ADS_GameModeBase::RemovePlayerSession for %s"), *Exiting->GetName() );
	}
#endif
}

void ADS_GameModeBase::SetClientInputEnabled(bool bEnabled)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		ADSPlayerController* DSPlayerController = Cast<ADSPlayerController>(Iterator->Get());
		if (IsValid(DSPlayerController))
		{
			DSPlayerController->Client_SetInputEnabled(bEnabled);
		}
	}
}

void ADS_GameModeBase::InitGameLift()
{
	if (UGameInstance* GameInstance = GetGameInstance(); IsValid(GameInstance))
	{
		if (DSGameInstanceSubsystem = GameInstance->GetSubsystem<UDS_GameInstanceSubsystem>(); IsValid(DSGameInstanceSubsystem))
		{
			FServerParameters ServerParameters;
			SetServerParameters(ServerParameters);
			DSGameInstanceSubsystem->InitGameLift(ServerParameters);
		}
	}
}
void ADS_GameModeBase::SetServerParameters(FServerParameters& OutServerParameters)
{
	//AuthToken returned from the "aws gamelift get-compute-auth-token" API. Note this will expire and require a new call to the API after 15 minutes.
	if (FParse::Value(FCommandLine::Get(), TEXT("-authtoken="), OutServerParameters.m_authToken))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("AUTH_TOKEN: %s"), *OutServerParameters.m_authToken)
	}

	//The Host/compute-name of the Amazon GameLift Servers Anywhere instance.
	if (FParse::Value(FCommandLine::Get(), TEXT("-hostid="), OutServerParameters.m_hostId))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("HOST_ID: %s"), *OutServerParameters.m_hostId)
	}

	//The Anywhere Fleet ID.
	if (FParse::Value(FCommandLine::Get(), TEXT("-fleetid="), OutServerParameters.m_fleetId))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("FLEET_ID: %s"), *OutServerParameters.m_fleetId)
	}

	//The WebSocket URL (GameLiftServiceSdkEndpoint).
	if (FParse::Value(FCommandLine::Get(), TEXT("-websocketurl="), OutServerParameters.m_webSocketUrl))
	{
		UE_LOG(LogDedicatedServers, Log, TEXT("WEBSOCKET_URL: %s"), *OutServerParameters.m_webSocketUrl)
	}

	FString glProcessId = "";
	if (FParse::Value(FCommandLine::Get(), TEXT("-processId="), glProcessId))
	{
		OutServerParameters.m_processId = TCHAR_TO_UTF8(*glProcessId);
	}
	else
	{
		// If no ProcessId is passed as a command line argument, generate a randomized unique string.
		FString TimeString = FString::FromInt(std::time(nullptr));
		FString ProcessId = "ProcessId_" + TimeString;
		OutServerParameters.m_processId = TCHAR_TO_UTF8(*ProcessId);
	}

	//The PID of the running process
	UE_LOG(LogDedicatedServers, Log, TEXT("PID: %s"), *OutServerParameters.m_processId);
}
