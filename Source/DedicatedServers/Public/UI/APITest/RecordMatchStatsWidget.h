#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RecordMatchStatsWidget.generated.h"

class UGameStatsManager;
class UButton;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API URecordMatchStatsWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_RecordMatchStats;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UGameStatsManager> GameStatsManagerClass;

private:

	UPROPERTY()
	TObjectPtr<UGameStatsManager> GameStatsManager;

	UFUNCTION()
	void RecordMatchStatsButtonClicked();
};
