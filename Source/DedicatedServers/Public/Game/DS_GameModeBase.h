#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Types/DSTypes.h"
#include "DS_GameModeBase.generated.h"

class UDS_GameInstanceSubsystem;
class APlayerController;
class APawn;
struct FServerParameters;

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADS_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "CharacterSpawn")
	void SpawnSelectedCharacter(APlayerController* PlayerController, TSubclassOf<APawn> PawnClassToSpawn);
	
protected:
	void StartCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle);
	void StopCountdownTimer(FCountdownTimerHandle& CountdownTimerHandle);
	void UpdateCountdownTimer(const FCountdownTimerHandle& CountdownTimerHandle);
	virtual void OnCountdownTimerFinished(ECountdownTimerType Type);
	virtual void InitSeamlessTravelPlayer(AController* NewController) override;
	virtual void PostSeamlessTravel() override;
	void TrySeamlessTravel(TSoftObjectPtr<UWorld> DestinationMap);
	void RemovePlayerSession(AController* Exiting);
	void SetClientInputEnabled(bool bEnabled);
	void InitGameLift();
	void SetServerParameters(FServerParameters& OutServerParameters);
	

	UFUNCTION(BlueprintImplementableEvent, Category="SeamlessTravel")
	void OnSeamlessTravel(AController* Controller);

	UFUNCTION(BlueprintImplementableEvent, Category="SeamlessTravel")
	void OnPostSeamlessTravel();


	
	UPROPERTY()
	TObjectPtr<UDS_GameInstanceSubsystem> DSGameInstanceSubsystem;


};
