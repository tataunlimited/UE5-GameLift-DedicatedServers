#include "UI/Portal/Dashboard/GamePage.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSLocalPlayerSubsystem.h"
#include "UI/GameSessions/GameSessionsManager.h"
#include "Save/ClientSaveManager.h"
#include "Player/PlayerTravelComponent.h" // New component for handling save RPCs
#include "UI/API/GameSessions/JoinGame.h"


void UGamePage::NativeConstruct()
{
	Super::NativeConstruct();

	GameSessionsManager = NewObject<UGameSessionsManager>(this, GameSessionsManagerClass);
	GameSessionsManager->BroadcastJoinGameSessionMessage.AddDynamic(this, &UGamePage::SetStatusMessage);

	JoinHomeArea->Button_JoinGame->OnClicked.AddDynamic(this, &UGamePage::JoinHomeAreaButtonClicked);
	JoinRaid->Button_JoinGame->OnClicked.AddDynamic(this, &UGamePage::JoinRaidButtonClicked);
	JoinTown->Button_JoinGame->OnClicked.AddDynamic(this, &UGamePage::JoinTownButtonClicked);
	CheckBox_AnywhereFleet->OnCheckStateChanged.AddDynamic(this, &UGamePage::AnyFleetStateChanged);

	// if is not editor disable checkbox
	if (!GIsEditor)
	{
		CheckBox_AnywhereFleet->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UGamePage::NativeDestruct()
{
	// Clean up delegates to prevent crashes
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (UPlayerTravelComponent* TravelComponent = PC->FindComponentByClass<UPlayerTravelComponent>())
		{
			TravelComponent->OnSaveComplete.RemoveAll(this);
		}
	}
	
	if (UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GameSessionsManager->GetDSLocalPlayerSubsystem())
	{
		if (UClientSaveManager* SaveManager = LocalPlayerSubsystem->GetSaveManager())
		{
			SaveManager->OnSaveToS3Complete.RemoveAll(this);
		}
	}
	Super::NativeDestruct();
}

// --- Button Handlers ---

void UGamePage::JoinRaidButtonClicked()
{
	InitiateTravel(ETravelDestination::Raid);
}

void UGamePage::JoinTownButtonClicked()
{
	InitiateTravel(ETravelDestination::Town);
}

void UGamePage::JoinHomeAreaButtonClicked()
{
	InitiateTravel(ETravelDestination::HomeArea);
}

// --- Core Travel & Save Logic ---

void UGamePage::InitiateTravel(ETravelDestination Destination)
{
	if (!GetWorld()) return;

	TargetDestination = Destination;
	SetButtonsEnabled(false);
	SetStatusMessage(TEXT("Saving player data..."), false);
	
	const FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(GetWorld());
	
	// Check if we are on a dedicated server map (Raid or Town)
	// NOTE: You might want a more robust way to check this, e.g., using Gameplay Tags on the World Settings.
	if (CurrentMapName.Contains(TEXT("Raid"), ESearchCase::IgnoreCase) || CurrentMapName.Contains(TEXT("Town"), ESearchCase::IgnoreCase))
	{
		APlayerController* PC = GetOwningPlayer();
		if (!PC)
		{
			UE_LOG(LogTemp, Error, TEXT("GamePage: Cannot get PlayerController to initiate server save."));
			FinalizeTravel(false);
			return;
		}

		// Ensure the player has the travel component
		UPlayerTravelComponent* TravelComponent = PC->FindComponentByClass<UPlayerTravelComponent>();
		if (!TravelComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("GamePage: PlayerController does not have a UPlayerTravelComponent. Adding one now."));
			TravelComponent = NewObject<UPlayerTravelComponent>(PC, TEXT("PlayerTravelComponent"));
			TravelComponent->RegisterComponent();
		}
		
		TravelComponent->OnSaveComplete.AddUniqueDynamic(this, &UGamePage::FinalizeTravel);
		TravelComponent->Server_RequestSavePlayerData();
	}
	// Check if we are on the local HomeArea map
	else if (CurrentMapName.Contains(TEXT("HomeArea"), ESearchCase::IgnoreCase))
	{
		APlayerController* PC = GetOwningPlayer();
		if (!PC)
		{
			UE_LOG(LogTemp, Error, TEXT("GamePage: Cannot get PlayerController to initiate client save."));
			FinalizeTravel(false);
			return;
		}
		UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GameSessionsManager->GetDSLocalPlayerSubsystem();
		if (LocalPlayerSubsystem && LocalPlayerSubsystem->GetSaveManager())
		{

			UClientSaveManager* SaveManager = LocalPlayerSubsystem->GetSaveManager();
			SaveManager->OnSaveToS3Complete.AddUniqueDynamic(this, &UGamePage::FinalizeTravel);

			AGameModeBase* CurrentGameMode = UGameplayStatics::GetGameMode(this);
			SaveManager->TriggerBlueprintClientSave(CurrentGameMode);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("GamePage: Could not get LocalPlayerSubsystem or SaveManager for local save."));
			FinalizeTravel(false);
		}
	}
	// Otherwise, we are likely in the Portal map or another non-gameplay map, so no save is needed.
	else
	{
		UE_LOG(LogTemp, Log, TEXT("GamePage: No save needed, proceeding directly to travel."));
		FinalizeTravel(true); // Treat as a "successful save" since none was required.
	}
}

void UGamePage::FinalizeTravel(bool bWasSaveSuccessful)
{
	// Unbind delegates to prevent them from being called multiple times
	NativeDestruct();

	if (!bWasSaveSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("GamePage: Pre-travel save failed, but proceeding with travel anyway."));
	}

	SetStatusMessage(TEXT("Traveling to destination..."), false);
	
	switch (TargetDestination)
	{
		case ETravelDestination::HomeArea:
			if (UWorld* World = GetWorld())
			{
				const FString CurrentMapName = UGameplayStatics::GetCurrentLevelName(World);
				const FString PortalMapAssetPath = GameSessionsManager->PortalMap.ToString();
				if (PortalMapAssetPath.Contains(CurrentMapName))
				{
					UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameSessionsManager->HomeAreaMap);
				}
				else
				{
					if (UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GameSessionsManager->GetDSLocalPlayerSubsystem())
					{
						LocalPlayerSubsystem->bShouldLoadHomeAreaLevel = true;
					}
					UGameplayStatics::OpenLevelBySoftObjectPtr(this, GameSessionsManager->PortalMap);
				}
			}
			break;
			
		case ETravelDestination::Raid:
			GameSessionsManager->JoinGameSession(TEXT("RAID"));
			break;
			
		case ETravelDestination::Town:
			GameSessionsManager->JoinGameSession(TEXT("TOWN"));
			break;
			
		case ETravelDestination::None:
		default:
			UE_LOG(LogTemp, Error, TEXT("GamePage: FinalizeTravel called with no destination!"));
			SetStatusMessage(TEXT("Error: No destination selected."), true);
			SetButtonsEnabled(true);
			break;
	}
}

// --- UI Helpers ---

void UGamePage::SetStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		SetButtonsEnabled(true);
	}
}

void UGamePage::SetButtonsEnabled(bool bEnabled)
{
	JoinHomeArea->Button_JoinGame->SetIsEnabled(bEnabled);
	JoinRaid->Button_JoinGame->SetIsEnabled(bEnabled);
	JoinTown->Button_JoinGame->SetIsEnabled(bEnabled);
}

void UGamePage::AnyFleetStateChanged(bool bIsChecked)
{
	GameSessionsManager->bIsAnyWhereFleet = bIsChecked;
}
