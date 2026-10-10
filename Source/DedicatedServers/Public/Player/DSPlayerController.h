#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DSPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTimerStateChangeDelegate, float, Time, ECountdownTimerType, Type);

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADSPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ADSPlayerController();
	virtual void ReceivedPlayer() override;
	virtual void OnRep_PlayerState() override;
	virtual void PostSeamlessTravel() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PawnLeavingGame() override;

		
	UFUNCTION(Client, Reliable)
	void Client_TimerUpdated(float CountDownTimeLeft, ECountdownTimerType Type) const;

	UFUNCTION(Client, Reliable)
	void Client_TimerStopped(float CountDownTimeLeft, ECountdownTimerType Type) const;

	UFUNCTION(Client, Reliable)
	void Client_SetInputEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetReady(bool bIsReady);

	UFUNCTION(Server, Reliable)
	void Server_SetVoiceStatus(bool bVoiceConnected, bool bIsSpeaking);

	UFUNCTION(BlueprintImplementableEvent)
	void OnBeforePawnLeavingGame();
	
	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChangeDelegate OnTimerUpdated;

	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChangeDelegate OnTimerStopped;

	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChangeDelegate OnTimerPaused;

	UPROPERTY(BlueprintAssignable)
	FOnTimerStateChangeDelegate OnTimerResumed;

	UPROPERTY(BlueprintReadOnly)
	FString Username;

	UPROPERTY(BlueprintReadOnly)
	FString PlayerSessionId;

	

	
protected:

	UFUNCTION(Server, Reliable)
	void Server_Ping(float TimeOfRequest);

	UFUNCTION(Client, Reliable)
	void Client_Pong(float TimeOfRequest);

private:

	float SingleTripTime;
};
