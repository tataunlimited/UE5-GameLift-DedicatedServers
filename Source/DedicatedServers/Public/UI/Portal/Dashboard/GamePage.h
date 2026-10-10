#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GamePage.generated.h"

class UCheckBox;
class UButton;
class UTextBlock;
class UGameSessionsManager;
class UJoinGame;
class UPlayerTravelComponent;

UENUM(BlueprintType)
enum class ETravelDestination : uint8
{
	None,
	HomeArea,
	Raid,
	Town
};

/**
 * Handles the UI for selecting a game map to travel to.
 * Implements pre-travel save logic based on the player's current map.
 */
UCLASS()
class DEDICATEDSERVERS_API UGamePage : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UJoinGame> JoinRaid;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UJoinGame> JoinTown;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UJoinGame> JoinHomeArea;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCheckBox> CheckBox_AnywhereFleet;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameSessionsManager> GameSessionsManagerClass;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:

	// --- Button Click Handlers ---
	UFUNCTION()
	void JoinRaidButtonClicked();

	UFUNCTION()
	void JoinTownButtonClicked();

	UFUNCTION(BlueprintCallable)
	void JoinHomeAreaButtonClicked();
	
	// --- Travel Logic ---

	/**
	 * Starts the travel process. Determines the current map and triggers the appropriate save mechanism.
	 * @param Destination The target map to travel to.
	 */
	void InitiateTravel(ETravelDestination Destination);

	/**
	 * Called after the save process is complete (or skipped). Executes the actual map travel.
	 * @param bWasSaveSuccessful Indicates the result of the pre-travel save. We travel regardless.
	 */
	UFUNCTION()
	void FinalizeTravel(bool bWasSaveSuccessful);

	// --- UI and State Management ---
	
	UFUNCTION()
	void SetStatusMessage(const FString& Message, bool bShouldResetWidgets);
	
	void SetButtonsEnabled(bool bEnabled);

	UFUNCTION()
	void AnyFleetStateChanged(bool bIsChecked);
	
	UPROPERTY()
	TObjectPtr<UGameSessionsManager> GameSessionsManager;

	// The destination the player has chosen. Stored while the async save operation is in progress.
	ETravelDestination TargetDestination = ETravelDestination::None;
};
