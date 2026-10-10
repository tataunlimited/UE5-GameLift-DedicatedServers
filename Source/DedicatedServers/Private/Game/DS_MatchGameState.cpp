#include "Game/DS_MatchGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

ADS_MatchGameState::ADS_MatchGameState()
{
	bReplicates = true;
	WorldSeed = -1; // default invalid
}

void ADS_MatchGameState::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority()) return;              // only run on server
	WorldSeed = FMath::Rand();               
	OnSeedGenerated.Broadcast(WorldSeed);
	UE_LOG(LogTemp, Warning, TEXT("ADS_MatchGameState::BeginPlay Seed: %d"), WorldSeed)

}

void ADS_MatchGameState::OnRep_WorldSeed()
{
	OnSeedGenerated.Broadcast(WorldSeed);
	UE_LOG(LogTemp, Warning, TEXT("ADS_MatchGameState::OnRep_Seed Seed: %d"), WorldSeed)

}

void ADS_MatchGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADS_MatchGameState, WorldSeed);
}
