#include "UI/APITest/RecordMatchStatsWidget.h"
#include "Components/Button.h"
#include "Player/DSLocalPlayerSubsystem.h"
#include "UI/GameStats/GameStatsManager.h"
#include "UI/HTTP/HTTPRequestTypes.h"

void URecordMatchStatsWidget::NativeConstruct()
{
	Super::NativeConstruct();
	GameStatsManager = NewObject<UGameStatsManager>(this, GameStatsManagerClass);
	Button_RecordMatchStats->OnClicked.AddDynamic(this, &URecordMatchStatsWidget::RecordMatchStatsButtonClicked);
}

void URecordMatchStatsWidget::RecordMatchStatsButtonClicked()
{
	check(GameStatsManager);

	const FString Username = TEXT("Taher");
	FDSRecordMatchStatsInput RecordMatchStatsInput;
	RecordMatchStatsInput.username = Username;
	RecordMatchStatsInput.matchStats.hits = 1;
	RecordMatchStatsInput.matchStats.headShotElims = 5;
	
	GameStatsManager->RecordMatchStats(RecordMatchStatsInput);
	
}
