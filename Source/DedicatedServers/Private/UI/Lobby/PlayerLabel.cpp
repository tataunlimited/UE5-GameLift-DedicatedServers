#include "UI/Lobby/PlayerLabel.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UPlayerLabel::SetUsername(const FString& Username) const
{
	TextBlock_Username->SetText(FText::FromString(Username));
}

FString UPlayerLabel::GetUsername() const
{
	return TextBlock_Username->GetText().ToString();
}

void UPlayerLabel::SetReadyStatus(bool bReady) const
{
	UE_LOG(LogTemp, Warning, TEXT("SetReadyStatus for %s: %s"), *GetUsername(), bReady ? TEXT("Ready") : TEXT("Not Ready"));

	TextBlock_Ready->SetText(bReady ? FText::FromString(TEXT("Ready")) : FText::GetEmpty());
}

void UPlayerLabel::SetVoiceStatus(bool bVoiceConnected, bool bIsSpeaking)
{
	if (!BackgroundImage) return;

	if (bVoiceConnected)
	{
		if (bIsSpeaking)
		{
			BackgroundImage->SetBrushTintColor(FLinearColor::Green); // Speaking
		}
		else
		{
			BackgroundImage->SetBrushTintColor(FLinearColor::Yellow); // Connected, not speaking
		}
	}
	else
	{
		BackgroundImage->SetBrushTintColor(FLinearColor::Gray); // Not connected
	}
}
