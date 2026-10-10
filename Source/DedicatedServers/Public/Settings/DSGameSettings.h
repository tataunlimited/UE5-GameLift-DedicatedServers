#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "DSGameSettings.generated.h"

class UClientSaveManager;
class UServerSaveManager;

/**
 * Defines the game-specific settings that can be configured in the Project Settings.
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Dedicated Server Game Settings"))
class DEDICATEDSERVERS_API UDSGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	// This property will now correctly appear in Project Settings under the
	// "Dedicated Server Game Settings" category!
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "SaveManager")
	TSoftClassPtr<UClientSaveManager> ClientSaveManagerClass;
	
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "SaveManager")
	TSoftClassPtr<UServerSaveManager> ServerSaveManagerClass;

	/** Application ID from the Discord Developer Portal, used for the Discord Social SDK OAuth flow. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Discord")
	int64 DiscordApplicationId = 0;
};