#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LeaderboardCard.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ULeaderboardCard : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetPlayerInfo(const FString& Username, const int32 Place, const int32 Wins) const;

private:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Username;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Place;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_Wins;
};
