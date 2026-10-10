#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CareerAchievement.generated.h"

class UImage;
class UTextBlock;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UCareerAchievement : public UUserWidget
{
	GENERATED_BODY()
public:
	void SetAchievementText(const FString& AchievementName, const int32 AchievementValue) const;
	void SetAchievementIcon(UTexture2D* Icon) const;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_AchievementName;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBlock_AchievementValue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Image_AchievementIcon;
};
