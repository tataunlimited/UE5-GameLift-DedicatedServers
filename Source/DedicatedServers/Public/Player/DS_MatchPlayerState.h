#pragma once

#include "CoreMinimal.h"
#include "DSPlayerState.h"
#include "DS_MatchPlayerState.generated.h"

struct FDSRecordMatchStatsInput;
class UGameStatsManager;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADS_MatchPlayerState : public ADSPlayerState
{
	GENERATED_BODY()
public:
	virtual void OnMatchEnded(const FString& Username);

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameStatsManager> GameStatsManagerClass;

protected:
	virtual void BeginPlay() override;
	void RecordMatchStats(const FDSRecordMatchStatsInput& RecordMatchStatsInput) const;


private:

	UPROPERTY()
	TObjectPtr<UGameStatsManager> GameStatsManager;
};
