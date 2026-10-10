#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerLabel.generated.h"

class UTextBlock;
class UImage;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UPlayerLabel : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetUsername(const FString& Username) const;
	FString GetUsername() const;
	void SetReadyStatus(bool bReady) const;
	void SetVoiceStatus(bool bVoiceConnected, bool bIsSpeaking);


private:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Username;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Ready;

	UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> BackgroundImage;
};
