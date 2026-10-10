#include "Player/PlayerTravelComponent.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Game/DS_GameInstanceSubsystem.h"
#include "Player/DSPlayerState.h"
#include "Save/ServerSaveManager.h"

class UDS_GameInstanceSubsystem;

UPlayerTravelComponent::UPlayerTravelComponent()
{
	SetIsReplicatedByDefault(true);
}

// --- Client -> Server ---

bool UPlayerTravelComponent::Server_RequestSavePlayerData_Validate()
{
	return true; // Add validation if needed
}

void UPlayerTravelComponent::Server_RequestSavePlayerData_Implementation()
{
	APlayerController* PC = Cast<APlayerController>(GetOwner());
	if (!PC || !PC->PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerTravelComponent: Server_RequestSavePlayerData called without a valid PlayerController or PlayerState."));
		Client_OnSavePlayerDataComplete(false); // Notify client of failure
		return;
	}

	ADSPlayerState* PlayerState = Cast<ADSPlayerState>(PC->PlayerState);
	if (!PlayerState)
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerTravelComponent: Server_RequestSavePlayerData called without a valid DSPlayerState."));
		Client_OnSavePlayerDataComplete(false);
		return;
	}
	PlayerUsername = PlayerState->GetDisplayName();

	UWorld* World = GetWorld();
	if (!World)
	{
		Client_OnSavePlayerDataComplete(false);
		return;
	}

	UDS_GameInstanceSubsystem* GameInstanceSubsystem = World->GetGameInstance()->GetSubsystem<UDS_GameInstanceSubsystem>();
	if (GameInstanceSubsystem && GameInstanceSubsystem->GetSaveManager())
	{
		UServerSaveManager* ServerSaveManager = GameInstanceSubsystem->GetSaveManager();
		
		ServerSaveManager->OnServerSaveToS3Complete.AddUniqueDynamic(this, &UPlayerTravelComponent::OnServerSaveCompleted);

		ServerSaveManager->TriggerBlueprintSaveForPlayer(PlayerUsername, PC->GetPawn());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerTravelComponent: Could not get GameInstanceSubsystem or ServerSaveManager."));
		Client_OnSavePlayerDataComplete(false);
	}
}

// --- Server -> Client ---

void UPlayerTravelComponent::OnServerSaveCompleted(bool bWasSuccessful, const FString& SavedUsername)
{
	if (SavedUsername == PlayerUsername)
	{
		if (GetWorld())
		{
			if(UDS_GameInstanceSubsystem* GI_Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<UDS_GameInstanceSubsystem>())
			{
				if (UServerSaveManager* SaveManager = GI_Subsystem->GetSaveManager())
				{
					SaveManager->OnServerSaveToS3Complete.RemoveDynamic(this, &UPlayerTravelComponent::OnServerSaveCompleted);
				}
			}
		}

		Client_OnSavePlayerDataComplete(bWasSuccessful);
	}
}

void UPlayerTravelComponent::Client_OnSavePlayerDataComplete_Implementation(bool bWasSuccessful)
{
	OnSaveComplete.Broadcast(bWasSuccessful);
}
