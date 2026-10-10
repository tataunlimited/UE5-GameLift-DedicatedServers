#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "JoinGame.generated.h"

class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UJoinGame : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UButton> Button_JoinGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_StatusMessage;
		
	UFUNCTION()
	void SetStatusMessage(const FString& Message, bool bShouldResetWidgets);

	
};
