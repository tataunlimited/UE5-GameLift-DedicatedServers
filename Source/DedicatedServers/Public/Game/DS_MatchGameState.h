#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "DS_MatchGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSeedGenerated, int32, Seed);


UCLASS()
class DEDICATEDSERVERS_API ADS_MatchGameState : public AGameStateBase
{
	GENERATED_BODY()
	
public:
	ADS_MatchGameState();

	virtual void BeginPlay() override;

	UPROPERTY(BlueprintAssignable, Category="Seed")
	FOnSeedGenerated OnSeedGenerated;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_WorldSeed)
	int32 WorldSeed;

	UFUNCTION()
	void OnRep_WorldSeed();
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
