#pragma once

#include "CoreMinimal.h"
#include "GameLiftServerSDK.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DS_GameInstanceSubsystem.generated.h"

class UServerSaveManager;

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UDS_GameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UDS_GameInstanceSubsystem();
	void InitGameLift(const FServerParameters& ServerParams);

	UPROPERTY(BlueprintReadOnly)
	bool bGameLiftInitialized;

	UFUNCTION(BlueprintPure, Category = "Player Session")
	UServerSaveManager* GetSaveManager() const { return ServerSaveManager; }
	

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

private:

	void ParseCommandLinePort(int32& OutPort);

	FProcessParameters ProcessParameters;

	UPROPERTY()
	TObjectPtr<UServerSaveManager> ServerSaveManager;
};
