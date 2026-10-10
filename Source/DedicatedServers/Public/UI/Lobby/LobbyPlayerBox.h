#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPlayerBox.generated.h"

struct FLobbyPlayerInfo;
class UPlayerLabel;
class UScrollBox;
class UButton;
class ALobbyState;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ULobbyPlayerBox : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollBox_PlayerInfo;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPlayerLabel> PlayerLabelClass;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Ready;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_Refresh;

	UFUNCTION(BlueprintCallable)
	void OnReadyClicked();


protected:
	virtual void NativeOnInitialized() override;
	void UpdatePlayerInfo(ALobbyState* LobbyState);

	UFUNCTION()
	void OnLobbyStateInitialized(ALobbyState* LobbyState);

	UFUNCTION()
	void CreateAndAddPlayerLabel(const FLobbyPlayerInfo& PlayerInfo);

	UFUNCTION()
	void OnPlayerRemoved(const FLobbyPlayerInfo& PlayerInfo);

	UFUNCTION()
	void OnPlayerReadyStatusChanged(const FLobbyPlayerInfo& PlayerInfo);

private:

	UPlayerLabel* FindPlayerLabel(const FString& Username);


	UFUNCTION()
	void SetupPlayerBox();

	UFUNCTION()
	void OnRefreshButtonClicked();


	bool bIsInitialized;
};



