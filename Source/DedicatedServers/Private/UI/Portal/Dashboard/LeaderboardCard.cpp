#include "UI/Portal/Dashboard/LeaderboardCard.h"

#include "Components/TextBlock.h"

void ULeaderboardCard::SetPlayerInfo(const FString& Username, const int32 Place, const int32 Wins) const
{
	TextBlock_Username->SetText(FText::FromString(Username));
	TextBlock_Place->SetText(FText::AsNumber(Place));
	TextBlock_Wins->SetText(FText::AsNumber(Wins));
}
