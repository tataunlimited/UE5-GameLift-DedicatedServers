#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Discord/UI/DiscordLobbyWidget.h"
#include "DiscordLobbyActor.generated.h"

UCLASS()
class DEDICATEDSERVERS_API ADiscordLobbyActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADiscordLobbyActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby Example")
	void ShowLobbyWidget();

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby Example")
	void HideLobbyWidget();

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby Example")
	void ConnectToDiscord();

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby Example")
	void DisconnectFromDiscord();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord Lobby Example")
	TSubclassOf<UDiscordLobbyWidget> LobbyWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord Lobby Example")
	FString LobbySecret = TEXT("my-game-lobby-secret");

private:
	UPROPERTY()
	UDiscordLobbyWidget* LobbyWidget;

	UPROPERTY()
	class UDiscordManager* DiscordManager;
};
