#include "Discord/DiscordManager.h"
#include "Engine/LocalPlayer.h"
#include "Player/DSPlayerController.h"
#include "Settings/DSGameSettings.h"

DEFINE_LOG_CATEGORY_STATIC(LogDiscordManager, Log, All);

int64 UDiscordManager::GetApplicationId()
{
    const UDSGameSettings* Settings = GetDefault<UDSGameSettings>();
    return Settings ? Settings->DiscordApplicationId : 0;
}

void UDiscordManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Discord = nullptr;
    CodeVerifier = nullptr;
    CurrentLobbyHandle = nullptr;
    CurrentLobbyId = FDiscordUniqueID();
    LobbyMembers.Empty();
    MemberHandles.Empty();
    bIsConnected = false;
    UE_LOG(LogDiscordManager, Warning, TEXT("UDiscordManager::Initialize called!"));

}

void UDiscordManager::ConnectToDiscord()
{
    if (bIsConnected)
    {
        UE_LOG(LogDiscordManager, Warning, TEXT("ConnectToDiscord called but already connected!"));
        return;
    }
    if (!GEngine) return;

    ULocalPlayer* LocalPlayer = GEngine->GetFirstGamePlayer(GetWorld());
    if (!LocalPlayer) return;

    Discord = ULocalPlayer::GetSubsystem<UDiscordLocalPlayerSubsystem>(LocalPlayer);
    if (!Discord) return;

    auto LogCallback = FDiscordClientLogCallback::CreateUObject(this, &UDiscordManager::OnLogMessage);
    FScriptDelegate StatusChanged;
    StatusChanged.BindUFunction(this, "OnStatusChanged");
    Discord->Client->AddLogCallback(LogCallback, EDiscordLoggingSeverity::Info);
    Discord->OnStatusChanged.Add(StatusChanged);

    // Set up lobby member callbacks
    Discord->Client->SetLobbyMemberAddedCallback(FDiscordClientLobbyMemberAddedCallback::CreateUObject(this, &UDiscordManager::OnLobbyMemberAdded));
    Discord->Client->SetLobbyMemberUpdatedCallback(FDiscordClientLobbyMemberUpdatedCallback::CreateUObject(this, &UDiscordManager::OnLobbyMemberUpdated));
    Discord->Client->SetLobbyMemberRemovedCallback(FDiscordClientLobbyMemberRemovedCallback::CreateUObject(this, &UDiscordManager::OnLobbyMemberRemoved));

    if (GetApplicationId() == 0)
    {
        UE_LOG(LogDiscordManager, Error, TEXT("Discord Application ID is not set. Configure it in Project Settings > Dedicated Server Game Settings."));
        return;
    }

    // Start the auth process
    CodeVerifier = Discord->Client->CreateAuthorizationCodeVerifier();
    auto AuthArgs = NewObject<UDiscordAuthorizationArgs>();
    AuthArgs->Init();
    AuthArgs->SetClientId(GetApplicationId());
    AuthArgs->SetScopes(UDiscordClient::GetDefaultCommunicationScopes());
    AuthArgs->SetCodeChallenge(CodeVerifier->Challenge());
    Discord->Client->Authorize(AuthArgs, FDiscordClientAuthorizationCallback::CreateUObject(this, &UDiscordManager::OnAuthorizeCompleted));
}

void UDiscordManager::DisconnectFromDiscord()
{
    UE_LOG(LogDiscordManager, Log, TEXT("🔄 Disconnecting from Discord..."));
    
    // End voice call if active
    if (CurrentCall)
    {
        UE_LOG(LogDiscordManager, Log, TEXT("🎤 Ending voice call..."));
        CurrentCall = nullptr;
    }
    
    // Leave lobby if in one
    if (CurrentLobbyHandle)
    {
        UE_LOG(LogDiscordManager, Log, TEXT("🚪 Leaving Discord lobby..."));
        CurrentLobbyHandle = nullptr;
        CurrentLobbyId = FDiscordUniqueID();
    }
    
    // Clear member data
    LobbyMembers.Empty();
    MemberHandles.Empty();
    
    // Disconnect from Discord client
    if (Discord && Discord->Client)
    {
        UE_LOG(LogDiscordManager, Log, TEXT("🔌 Disconnecting Discord client..."));
        Discord->Client->Disconnect();
    }
    
    // Clear Discord subsystem reference
    Discord = nullptr;
    CodeVerifier = nullptr;
    bIsConnected = false;
    OnDiscordConnectionUpdated.Broadcast(bIsConnected);

    if (GEngine)
    {
        APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
        if (ADSPlayerController* DSController = Cast<ADSPlayerController>(PC))
        {
            // Assume voice is always connected if we get this event
            DSController->Server_SetVoiceStatus(false, false);
        }
    }

    UE_LOG(LogDiscordManager, Log, TEXT("✅ Successfully disconnected from Discord"));
}

void UDiscordManager::OnLogMessage(FString Message, EDiscordLoggingSeverity Severity)
{
    UE_LOG(LogDiscordManager, Log, TEXT("[%s] %s"), *UEnum::GetValueAsString(Severity), *Message);
}

void UDiscordManager::OnStatusChanged(EDiscordClientStatus Status, EDiscordClientError Error, int32 ErrorDetail)
{
    UE_LOG(LogDiscordManager, Log, TEXT("Connection status: %s"), *UEnum::GetValueAsString(Status));
    if (Status == EDiscordClientStatus::Ready)
    {
        bIsConnected = true;
        // Set LocalUserId from Discord client
        if (Discord && Discord->Client)
        {
            if (UDiscordUserHandle* CurrentUser = Discord->Client->GetCurrentUser())
            {
                
                LocalUserId = CurrentUser->Id();
                UE_LOG(LogDiscordManager, Log, TEXT("Local Discord UserId: %s"), *LocalUserId.ToString());
            }
        }
        UE_LOG(LogDiscordManager, Log, TEXT("Connected to Discord! Ready to go!"));
        UE_LOG(LogDiscordManager, Log, TEXT("👥 Friends Count: %d"), Discord->Client->GetRelationships().Num());
        UDiscordActivity* Activity = NewObject<UDiscordActivity>();
        Activity->Init();
        Activity->SetType(EDiscordActivityTypes::Playing);
        // Activity->SetState("In competitive match");
        // Activity->SetDetails("Rank: Diamond II");
        Discord->Client->UpdateRichPresence(Activity, FDiscordClientUpdateRichPresenceCallback::CreateUObject(this, &UDiscordManager::OnRichPresenceUpdated));
        CreateLobby();
        OnDiscordConnectionUpdated.Broadcast(bIsConnected);
    }
    else if (Error != EDiscordClientError::None)
    {
        UE_LOG(LogDiscordManager, Error, TEXT("Connection error: %s (Detail: %d)"), *UEnum::GetValueAsString(Error), ErrorDetail);
    }
}

void UDiscordManager::OnRichPresenceUpdated(UDiscordClientResult* Result)
{
    if (Result->Successful()) {
        UE_LOG(LogDiscordManager, Log, TEXT("Rich Presence updated successfully!"));
    } else {
        UE_LOG(LogDiscordManager, Error, TEXT("Rich Presence update failed"));
    }
}

void UDiscordManager::OnAuthorizeCompleted(UDiscordClientResult* Result, FString Code, FString RedirectUri)
{
    if (!Result->Successful())
    {
        UE_LOG(LogDiscordManager, Error, TEXT("Discord authorization failed: %s"), *Result->Error());
        return;
    }
    UE_LOG(LogDiscordManager, Log, TEXT("Authorization successful! Getting access token..."));
    Discord->Client->GetToken(GetApplicationId(), Code, CodeVerifier->Verifier(), RedirectUri,
        FDiscordClientTokenExchangeCallback::CreateUObject(this, &UDiscordManager::OnTokenExchange));
}

void UDiscordManager::OnTokenExchange(UDiscordClientResult* Result, FString AccessToken, FString RefreshToken, EDiscordAuthorizationTokenType TokenType, int32 ExpiresIn, FString Scope)
{
    if (!Result->Successful())
    {
        UE_LOG(LogDiscordManager, Error, TEXT("Discord token exchange failed: %s"), *Result->Error());
        return;
    }
    Discord->Client->UpdateToken(TokenType, AccessToken,
        FDiscordClientUpdateTokenCallback::CreateUObject(this, &UDiscordManager::OnTokenUpdated));
}

void UDiscordManager::OnTokenUpdated(UDiscordClientResult* Result)
{
    if (!Result->Successful())
    {
        UE_LOG(LogDiscordManager, Error, TEXT("Discord token update failed: %s"), *Result->Error());
        return;
    }
    Discord->Client->Connect();
}

void UDiscordManager::CreateLobby()
{
    if (!Discord || !Discord->Client) return;
    Discord->Client->CreateOrJoinLobby(
           LobbySecret,
           FDiscordClientCreateOrJoinLobbyCallback::CreateUObject(this, &UDiscordManager::OnCreateOrJoinLobbyCompleted)
       );
}

void UDiscordManager::OnCreateOrJoinLobbyCompleted(UDiscordClientResult* Result, FDiscordUniqueID LobbyID)
{
    if (!Result || !Result->Successful())
    {
        UE_LOG(LogDiscordManager, Error, TEXT("❌ Failed to join/create lobby: %s"), Result ? *Result->Error() : TEXT("Unknown error"));
        return;
    }

    UE_LOG(LogDiscordManager, Log, TEXT("🎮 Successfully joined/created lobby! LobbyID: %s"), *LobbyID.ToString());
    
    // Store lobby information
    CurrentLobbyId = LobbyID;
    CurrentLobbyHandle = Discord->Client->GetLobbyHandle(LobbyID);
    
    UE_LOG(LogDiscordManager, Log, TEXT("📋 Stored lobby info - ID: %s"), *LobbyID.ToString());
    
    // Refresh lobby members
    RefreshLobbyMembers();
    
    StartVoiceCall(LobbyID);
}

void UDiscordManager::StartVoiceCall(FDiscordUniqueID LobbyID)
{
    if (!Discord || !Discord->Client) return;
    
    // StartCall returns a UDiscordCall* (null if already in call)
    CurrentCall = Discord->Client->StartCall(LobbyID);

    if (CurrentCall)
    {
        UE_LOG(LogDiscordManager, Log, TEXT("🎤 Voice call operation initiated..."));
        
        // Set up speaking status callback
        CurrentCall->SetSpeakingStatusChangedCallback(FDiscordCallOnSpeakingStatusChanged::CreateUObject(this, &UDiscordManager::OnSpeakingStatusChanged));
    }
    else
    {
        UE_LOG(LogDiscordManager, Log, TEXT("ℹ️ Already in this voice channel"));
    }
}

// New lobby member tracking functionality
void UDiscordManager::RefreshLobbyMembers()
{
    if (!CurrentLobbyHandle)
    {
        UE_LOG(LogDiscordManager, Warning, TEXT("No current lobby handle to refresh members"));
        return;
    }

    // Clear existing members
    LobbyMembers.Empty();
    MemberHandles.Empty();

    UE_LOG(LogDiscordManager, Log, TEXT("🔄 Refreshing lobby members..."));
    
    // Add current user as a member for testing
    if (Discord && Discord->Client)
    {
        // Try to get the current user from the voice call
        if (CurrentCall && CurrentLobbyHandle)
        {
            for (auto Member : CurrentLobbyHandle->LobbyMembers())
            {
                
                UE_LOG(LogDiscordManager, Log, TEXT("➕ Try to add a member: %s (%s)"), *Member->GetFullName(), *Member->GetName());

                FDiscordUniqueID MemberId = Member->Id();
                UDiscordUserHandle* UserHandle = Discord->Client->GetUser(MemberId);
                
                if (UserHandle)
                {
                    FLobbyMemberInfo CurrentMemberInfo;
                    CurrentMemberInfo.UserId = MemberId;
                    CurrentMemberInfo.DisplayName = UserHandle->DisplayName();
                    CurrentMemberInfo.Username = UserHandle->Username();
                    CurrentMemberInfo.bIsSpeaking = false;
                    CurrentMemberInfo.bIsMuted = false;
                    CurrentMemberInfo.bIsDeafened = false;
                    CurrentMemberInfo.Volume = 1.0f;
                
                    LobbyMembers.Add(CurrentMemberInfo.UserId, CurrentMemberInfo);
                    UE_LOG(LogDiscordManager, Log, TEXT("➕ Added member: %s (%s)"), *CurrentMemberInfo.DisplayName, *CurrentMemberInfo.Username);
                    BroadcastMembersChanged();
                }
                else
                {
                    UE_LOG(LogDiscordManager, Warning, TEXT("❌ Could not get user handle for member %s"), *MemberId.ToString());
                }
            }
        }
        else
        {
            UE_LOG(LogDiscordManager, Warning, TEXT("❌ No current call available"));
        }
    }
}

TArray<FLobbyMemberInfo> UDiscordManager::GetLobbyMembers() const
{
    TArray<FLobbyMemberInfo> Members;
    LobbyMembers.GenerateValueArray(Members);
    return Members;
}

FLobbyMemberInfo UDiscordManager::GetMemberInfo(FDiscordUniqueID UserId) const
{
    if (const FLobbyMemberInfo* MemberInfo = LobbyMembers.Find(UserId))
    {
        return *MemberInfo;
    }
    
    return FLobbyMemberInfo();
}

bool UDiscordManager::IsMemberSpeaking(FDiscordUniqueID UserId) const
{
    if (const FLobbyMemberInfo* MemberInfo = LobbyMembers.Find(UserId))
    {
        return MemberInfo->bIsSpeaking;
    }
    return false;
}

bool UDiscordManager::IsMemberMuted(FDiscordUniqueID UserId) const
{
    if (const FLobbyMemberInfo* MemberInfo = LobbyMembers.Find(UserId))
    {
        return MemberInfo->bIsMuted;
    }
    return false;
}

float UDiscordManager::GetMemberVolume(FDiscordUniqueID UserId) const
{
    if (const FLobbyMemberInfo* MemberInfo = LobbyMembers.Find(UserId))
    {
        return MemberInfo->Volume;
    }
    return 1.0f;
}

// Lobby member event callbacks
void UDiscordManager::OnLobbyMemberAdded(FDiscordUniqueID LobbyId, FDiscordUniqueID UserId)
{
    UE_LOG(LogDiscordManager, Log, TEXT("👤 Member added to lobby: %s"), *UserId.ToString());
    UE_LOG(LogDiscordManager, Log, TEXT("Current Lobby ID: %s"), *CurrentLobbyId.ToString());
    UE_LOG(LogDiscordManager, Log, TEXT("Incoming Lobby ID: %s"), *LobbyId.ToString());
    
    if (LobbyId == CurrentLobbyId)
    {
        UE_LOG(LogDiscordManager, Log, TEXT("✅ Lobby IDs match, updating member info"));
        UpdateMemberInfo(UserId);
    }
    else
    {
        UE_LOG(LogDiscordManager, Log, TEXT("❌ Lobby IDs don't match"));
    }
}

void UDiscordManager::OnLobbyMemberUpdated(FDiscordUniqueID LobbyId, FDiscordUniqueID UserId)
{
    UE_LOG(LogDiscordManager, Log, TEXT("🔄 Member updated in lobby: %s"), *UserId.ToString());
    RefreshLobbyMembers();
    if (LobbyId == CurrentLobbyId)
    {
        UpdateMemberInfo(UserId);
    }
}

void UDiscordManager::OnLobbyMemberRemoved(FDiscordUniqueID LobbyId, FDiscordUniqueID UserId)
{
    UE_LOG(LogDiscordManager, Log, TEXT("👋 Member removed from lobby: %s"), *UserId.ToString());
    
    if (LobbyId == CurrentLobbyId)
    {
        RemoveMemberInfo(UserId);
    }
}

// Voice call event callbacks
void UDiscordManager::OnSpeakingStatusChanged(FDiscordUniqueID UserId, bool bIsSpeaking)
{
    if (UserId != LocalUserId)
        return;
    UE_LOG(LogDiscordManager, Log, TEXT("🗣️ Local user speaking status changed: %s"), bIsSpeaking ? TEXT("Speaking") : TEXT("Not Speaking"));
    // Update member info
    if (FLobbyMemberInfo* MemberInfo = LobbyMembers.Find(UserId))
    {
        MemberInfo->bIsSpeaking = bIsSpeaking;
        BroadcastMembersChanged();
    }
    // Call Server_SetVoiceStatus on local DSPlayerController
    if (GEngine)
    {
        APlayerController* PC = GEngine->GetFirstLocalPlayerController(GetWorld());
        if (ADSPlayerController* DSController = Cast<ADSPlayerController>(PC))
        {
            // Assume voice is always connected if we get this event
            DSController->Server_SetVoiceStatus(true, bIsSpeaking);
        }
    }
    // Broadcast the event
    OnMemberSpeakingStatusChanged.Broadcast(UserId, bIsSpeaking);
}



// Helper functions
void UDiscordManager::UpdateMemberInfo(FDiscordUniqueID UserId)
{
    if (!CurrentLobbyHandle) return;
    
    // Get member handle
    UDiscordLobbyMemberHandle* MemberHandle = CurrentLobbyHandle->GetLobbyMemberHandle(UserId);
    if (!MemberHandle) return;
    
    // Store the member handle
    MemberHandles.Add(UserId, MemberHandle);
    
    // Get user information
    UDiscordUserHandle* UserHandle = Discord->Client->GetUser(UserId);
    if (UserHandle)
    {
        FLobbyMemberInfo MemberInfo;
        MemberInfo.UserId = UserId;
        MemberInfo.DisplayName = UserHandle->DisplayName();
        MemberInfo.Username = UserHandle->Username();
        
        // Initialize voice state
        MemberInfo.bIsSpeaking = false;
        MemberInfo.bIsMuted = false;
        MemberInfo.bIsDeafened = false;
        MemberInfo.Volume = 1.0f;
        
        LobbyMembers.Add(UserId, MemberInfo);
        
        UE_LOG(LogDiscordManager, Log, TEXT("📝 Updated member info for: %s (%s)"), *MemberInfo.DisplayName, *MemberInfo.Username);
        
        BroadcastMembersChanged();
    }
}

void UDiscordManager::RemoveMemberInfo(FDiscordUniqueID UserId)
{
    LobbyMembers.Remove(UserId);
    MemberHandles.Remove(UserId);
    
    UE_LOG(LogDiscordManager, Log, TEXT("🗑️ Removed member info for: %s"), *UserId.ToString());
    
    BroadcastMembersChanged();
}

void UDiscordManager::ForceRefreshMembers()
{
    UE_LOG(LogDiscordManager, Log, TEXT("🔄 Force refreshing members..."));
    RefreshLobbyMembers();
}

void UDiscordManager::BroadcastMembersChanged()
{
    TArray<FLobbyMemberInfo> Members = GetLobbyMembers();
    OnLobbyMembersChanged.Broadcast(Members);
    
    UE_LOG(LogDiscordManager, Log, TEXT("📢 Broadcasted member list update. Total members: %d"), CurrentLobbyHandle->LobbyMembers().Num());
}
