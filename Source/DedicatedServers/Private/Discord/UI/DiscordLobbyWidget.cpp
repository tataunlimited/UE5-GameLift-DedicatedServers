#include "Discord/UI/DiscordLobbyWidget.h"
#include "Discord/DiscordManager.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Engine/GameInstance.h"

void UDiscordMemberWidget::NativeConstruct()
{
    Super::NativeConstruct();
}

void UDiscordMemberWidget::SetMemberInfo(const FLobbyMemberInfo& MemberInfo)
{
    CurrentMemberInfo = MemberInfo;
    
    if (MemberNameText)
    {
        MemberNameText->SetText(FText::FromString(MemberInfo.DisplayName));
    }
    
    if (UsernameText)
    {
        UsernameText->SetText(FText::FromString(MemberInfo.Username));
    }
    
    UpdateSpeakingStatus(MemberInfo.bIsSpeaking);
    UpdateMuteStatus(MemberInfo.bIsMuted);
}

void UDiscordMemberWidget::UpdateSpeakingStatus(bool bIsSpeaking)
{
    if (SpeakingIndicator)
    {
        SpeakingIndicator->SetVisibility(bIsSpeaking ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    
    if (MemberBorder)
    {
        FLinearColor BorderColor = bIsSpeaking ? SpeakingColor : NormalColor;
        MemberBorder->SetBrushColor(BorderColor);
    }
}

void UDiscordMemberWidget::UpdateMuteStatus(bool bIsMuted)
{
    if (MuteIndicator)
    {
        MuteIndicator->SetVisibility(bIsMuted ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
    
    // if (MemberNameText)
    // {
    //     FLinearColor TextColor = bIsMuted ? MutedColor : NormalColor;
    //     MemberNameText->SetColorAndOpacity(TextColor);
    // }
    //
    // if (UsernameText)
    // {
    //     FLinearColor TextColor = bIsMuted ? MutedColor : NormalColor;
    //     UsernameText->SetColorAndOpacity(TextColor);
    // }
}

// UDiscordLobbyWidget Implementation
void UDiscordLobbyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // Get the Discord Manager
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        DiscordManager = GameInstance->GetSubsystem<UDiscordManager>();
    }
    
    InitializeWidget();
}

void UDiscordLobbyWidget::NativeDestruct()
{
    // Unbind delegates
    if (DiscordManager)
    {
        DiscordManager->OnLobbyMembersChanged.RemoveAll(this);
        DiscordManager->OnMemberSpeakingStatusChanged.RemoveAll(this);
    }
    
    Super::NativeDestruct();
}

void UDiscordLobbyWidget::InitializeWidget()
{
    if (!DiscordManager)
    {
        UE_LOG(LogTemp, Warning, TEXT("DiscordLobbyWidget: DiscordManager not found"));
        return;
    }
    
    // Bind to Discord Manager events
    DiscordManager->OnLobbyMembersChanged.AddDynamic(this, &UDiscordLobbyWidget::OnLobbyMembersChanged);
    DiscordManager->OnMemberSpeakingStatusChanged.AddDynamic(this, &UDiscordLobbyWidget::OnMemberSpeakingStatusChanged);
    
    // Set initial title
    if (LobbyTitleText)
    {
        LobbyTitleText->SetText(FText::FromString(TEXT("Discord Lobby")));
    }
    
    // Refresh the member list
    RefreshMemberList();
}

void UDiscordLobbyWidget::RefreshMemberList()
{
    if (!DiscordManager)
    {
        return;
    }
    
    // Get current members
    TArray<FLobbyMemberInfo> Members = DiscordManager->GetLobbyMembers();
    
    // Update the display
    for (const FLobbyMemberInfo& MemberInfo : Members)
    {
        UpdateMemberWidget(MemberInfo.UserId, MemberInfo);
    }
    
    // Remove widgets for members that are no longer in the lobby
    TArray<FDiscordUniqueID> MemberIdsToRemove;
    for (const auto& Pair : MemberWidgets)
    {
        bool bFound = false;
        for (const FLobbyMemberInfo& MemberInfo : Members)
        {
            if (MemberInfo.UserId == Pair.Key)
            {
                bFound = true;
                break;
            }
        }
        
        if (!bFound)
        {
            MemberIdsToRemove.Add(Pair.Key);
        }
    }
    
    for (FDiscordUniqueID UserId : MemberIdsToRemove)
    {
        RemoveMemberWidget(UserId);
    }
    
    UpdateMemberCount();
}

void UDiscordLobbyWidget::OnLobbyMembersChanged(const TArray<FLobbyMemberInfo>& Members)
{
    // Clear existing widgets
    for (const auto& Pair : MemberWidgets)
    {
        if (Pair.Value)
        {
            Pair.Value->RemoveFromParent();
        }
    }
    MemberWidgets.Empty();
    
    // Create widgets for each member
    for (const FLobbyMemberInfo& MemberInfo : Members)
    {
        UpdateMemberWidget(MemberInfo.UserId, MemberInfo);
    }
    
    UpdateMemberCount();
}

void UDiscordLobbyWidget::OnMemberSpeakingStatusChanged(FDiscordUniqueID UserId, bool bIsSpeaking)
{
    if (UDiscordMemberWidget* MemberWidget = MemberWidgets.FindRef(UserId))
    {
        MemberWidget->UpdateSpeakingStatus(bIsSpeaking);
    }
}

void UDiscordLobbyWidget::UpdateMemberWidget(FDiscordUniqueID UserId, const FLobbyMemberInfo& MemberInfo)
{
    if (!MembersList || !MemberWidgetClass)
    {
        return;
    }
    
    // Create or update member widget
    UDiscordMemberWidget* MemberWidget = MemberWidgets.FindRef(UserId);
    if (!MemberWidget)
    {
        MemberWidget = CreateWidget<UDiscordMemberWidget>(this, MemberWidgetClass);
        if (MemberWidget)
        {
            MembersList->AddChild(MemberWidget);
            MemberWidgets.Add(UserId, MemberWidget);
        }
    }
    
    if (MemberWidget)
    {
        MemberWidget->SetMemberInfo(MemberInfo);
    }
}

void UDiscordLobbyWidget::RemoveMemberWidget(FDiscordUniqueID UserId)
{
    if (UDiscordMemberWidget* MemberWidget = MemberWidgets.FindRef(UserId))
    {
        MemberWidget->RemoveFromParent();
        MemberWidgets.Remove(UserId);
    }
}

void UDiscordLobbyWidget::UpdateMemberCount()
{
    if (MemberCountText)
    {
        int32 MemberCount = MemberWidgets.Num();
        FString CountText = FString::Printf(TEXT("Members: %d"), MemberCount);
        MemberCountText->SetText(FText::FromString(CountText));
    }
} 