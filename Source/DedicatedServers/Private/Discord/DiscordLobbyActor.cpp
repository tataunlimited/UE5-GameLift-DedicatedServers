#include "Discord/DiscordLobbyActor.h"
#include "Discord/DiscordManager.h"
#include "Discord/UI/DiscordLobbyWidget.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"


// Sets default values
ADiscordLobbyActor::ADiscordLobbyActor()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ADiscordLobbyActor::BeginPlay()
{
	Super::BeginPlay();

	ENetMode CurrentNetMode = GetNetMode();

	if (CurrentNetMode == NM_DedicatedServer)
	{
		return;
	}
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		DiscordManager = GameInstance->GetSubsystem<UDiscordManager>();
	}
    
	// Auto-connect to Discord when the game starts
	//ConnectToDiscord();
	
}

void ADiscordLobbyActor::ShowLobbyWidget()
{
	if (!LobbyWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordLobbyExample: LobbyWidgetClass not set"));
		return;
	}
    
	if (!LobbyWidget)
	{
		LobbyWidget = CreateWidget<UDiscordLobbyWidget>(GetWorld(), LobbyWidgetClass);
	}
    
	if (LobbyWidget)
	{
		LobbyWidget->AddToViewport();
		UE_LOG(LogTemp, Log, TEXT("DiscordLobbyExample: Lobby widget shown"));
	}
}

void ADiscordLobbyActor::HideLobbyWidget()
{
	if (LobbyWidget)
	{
		LobbyWidget->RemoveFromParent();
		UE_LOG(LogTemp, Log, TEXT("DiscordLobbyExample: Lobby widget hidden"));
	}
}

void ADiscordLobbyActor::ConnectToDiscord()
{
	if (DiscordManager)
	{
		DiscordManager->ConnectToDiscord();
		UE_LOG(LogTemp, Log, TEXT("DiscordLobbyExample: Connecting to Discord..."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordLobbyExample: DiscordManager not found"));
	}
} 

void ADiscordLobbyActor::DisconnectFromDiscord()
{
	if (DiscordManager)
	{
		DiscordManager->DisconnectFromDiscord();
		UE_LOG(LogTemp, Log, TEXT("DiscordLobbyExample: Disconnecting from Discord..."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("DiscordLobbyExample: DiscordManager not found"));
	}
} 