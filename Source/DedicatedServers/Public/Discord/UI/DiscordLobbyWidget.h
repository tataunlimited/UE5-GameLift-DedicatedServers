#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Discord/DiscordManager.h"
#include "DiscordLobbyWidget.generated.h"


class UDiscordManager;

// Widget for displaying a single lobby member
UCLASS()
class DEDICATEDSERVERS_API UDiscordMemberWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Discord Member Widget")
	void SetMemberInfo(const FLobbyMemberInfo& MemberInfo);

	UFUNCTION(BlueprintCallable, Category = "Discord Member Widget")
	void UpdateSpeakingStatus(bool bIsSpeaking);

	UFUNCTION(BlueprintCallable, Category = "Discord Member Widget")
	void UpdateMuteStatus(bool bIsMuted);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MemberNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* UsernameText;

	UPROPERTY(meta = (BindWidget))
	UImage* SpeakingIndicator;

	UPROPERTY(meta = (BindWidget))
	UImage* MuteIndicator;

	UPROPERTY(meta = (BindWidget))
	UBorder* MemberBorder;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord Member Widget")
	FLinearColor SpeakingColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord Member Widget")
	FLinearColor NormalColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord Member Widget")
	FLinearColor MutedColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

private:
	FLobbyMemberInfo CurrentMemberInfo;
};

// Main lobby widget
UCLASS()
class DEDICATEDSERVERS_API UDiscordLobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Discord Lobby Widget")
	void InitializeWidget();

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby Widget")
	void RefreshMemberList();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnLobbyMembersChanged(const TArray<FLobbyMemberInfo>& Members);

	UFUNCTION()
	void OnMemberSpeakingStatusChanged(FDiscordUniqueID UserId, bool bIsSpeaking);

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* MembersList;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* LobbyTitleText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MemberCountText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Discord Lobby Widget")
	TSubclassOf<UDiscordMemberWidget> MemberWidgetClass;

private:
	UPROPERTY()
	UDiscordManager* DiscordManager;

	UPROPERTY()
	TMap<FDiscordUniqueID, UDiscordMemberWidget*> MemberWidgets;

	void UpdateMemberWidget(FDiscordUniqueID UserId, const FLobbyMemberInfo& MemberInfo);
	void RemoveMemberWidget(FDiscordUniqueID UserId);
	void UpdateMemberCount();
};
