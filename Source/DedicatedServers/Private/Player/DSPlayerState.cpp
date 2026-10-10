#include "Player/DSPlayerState.h"

#include "Net/UnrealNetwork.h"

FString ADSPlayerState::GetDisplayName()
{
	return DisplayName;
}

void ADSPlayerState::SetDisplayName(const FString& InUsername)
{
	DisplayName = InUsername;
}

void ADSPlayerState::OnRep_DisplayName()
{
	OnDisplayNameUpdated.Broadcast(DisplayName);
	UE_LOG(LogTemp, Warning, TEXT("DisplayName replicated: %s"), *DisplayName);

}

void ADSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADSPlayerState, DisplayName);
	DOREPLIFETIME(ADSPlayerState, SelectedCharacterClass);
}

void ADSPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	if (ADSPlayerState* DSPlayerState = Cast<ADSPlayerState>(PlayerState))
	{
		DSPlayerState->DisplayName = DisplayName;
		DSPlayerState->SelectedCharacterClass = SelectedCharacterClass;
	}
}