#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DiscordLocalPlayerSubsystem.h"
#include "DiscordManager.generated.h"


class UDiscordLobbyHandle;
class UDiscordLobbyMemberHandle;
class UDiscordCall;

// Structure to hold lobby member information
USTRUCT(BlueprintType)
struct FLobbyMemberInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FDiscordUniqueID UserId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FString Username;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bIsSpeaking;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bIsMuted;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	bool bIsDeafened;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	float Volume;

	FLobbyMemberInfo()
	{
		UserId = FDiscordUniqueID();
		DisplayName = TEXT("");
		Username = TEXT("");
		bIsSpeaking = false;
		bIsMuted = false;
		bIsDeafened = false;
		Volume = 1.0f;
	}
};

// Delegate for when lobby members change
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLobbyMembersChanged, const TArray<FLobbyMemberInfo>&, Members);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiscordConnectionUpdated, bool, bIsConnected);

// Delegate for when a specific member's speaking status changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMemberSpeakingStatusChanged, FDiscordUniqueID, UserId, bool, bIsSpeaking);

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UDiscordManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable)
	void ConnectToDiscord();

	UFUNCTION(BlueprintCallable)
	void DisconnectFromDiscord();

	UFUNCTION(BlueprintCallable)
	bool IsConnected() const { return bIsConnected; }

	// Discord callbacks
	UFUNCTION()
	void OnLogMessage(FString Message, EDiscordLoggingSeverity Severity);

	UFUNCTION()
	void OnStatusChanged(EDiscordClientStatus Status, EDiscordClientError Error, int32 ErrorDetail);

	UFUNCTION()
	void OnAuthorizeCompleted(UDiscordClientResult* Result, FString Code, FString RedirectUri);

	UFUNCTION()
	void OnTokenExchange(UDiscordClientResult* Result, FString AccessToken, FString RefreshToken, EDiscordAuthorizationTokenType TokenType, int32 ExpiresIn, FString Scope);

	UFUNCTION()
	void OnTokenUpdated(UDiscordClientResult* Result);

	UFUNCTION()
	void OnRichPresenceUpdated(UDiscordClientResult* Result);

	UFUNCTION(BlueprintCallable)
	void CreateLobby();

	void OnCreateOrJoinLobbyCompleted(UDiscordClientResult* Result, FDiscordUniqueID LobbyID);

	UFUNCTION(BlueprintCallable)
	void StartVoiceCall(FDiscordUniqueID LobbyID);

	UPROPERTY()
	UDiscordCall* CurrentCall;

	// New lobby member tracking functionality
	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	void RefreshLobbyMembers();

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	void ForceRefreshMembers();

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	TArray<FLobbyMemberInfo> GetLobbyMembers() const;

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	FLobbyMemberInfo GetMemberInfo(FDiscordUniqueID UserId) const;

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	bool IsMemberSpeaking(FDiscordUniqueID UserId) const;

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	bool IsMemberMuted(FDiscordUniqueID UserId) const;

	UFUNCTION(BlueprintCallable, Category = "Discord Lobby")
	float GetMemberVolume(FDiscordUniqueID UserId) const;

	// Lobby member event callbacks
	UFUNCTION()
	void OnLobbyMemberAdded(FDiscordUniqueID LobbyId, FDiscordUniqueID UserId);

	UFUNCTION()
	void OnLobbyMemberUpdated(FDiscordUniqueID LobbyId, FDiscordUniqueID UserId);

	UFUNCTION()
	void OnLobbyMemberRemoved(FDiscordUniqueID LobbyId, FDiscordUniqueID UserId);

	// Voice call event callbacks
	UFUNCTION()
	void OnSpeakingStatusChanged(FDiscordUniqueID UserId, bool bIsSpeaking);

	UPROPERTY(BlueprintReadWrite)
	FString LobbySecret;

	// Blueprint events
	UPROPERTY(BlueprintAssignable, Category = "Discord Lobby")
	FOnLobbyMembersChanged OnLobbyMembersChanged;

	UPROPERTY(BlueprintAssignable, Category = "Discord Voice")
	FOnMemberSpeakingStatusChanged OnMemberSpeakingStatusChanged;

	UPROPERTY(BlueprintAssignable, Category = "Discord Voice")
	FOnDiscordConnectionUpdated OnDiscordConnectionUpdated;

private:
	bool bIsConnected = false;
	/** Discord Application ID, read from Project Settings > Dedicated Server Game Settings. */
	static int64 GetApplicationId();
	UPROPERTY()
	UDiscordLocalPlayerSubsystem* Discord;

	UPROPERTY()
	UDiscordAuthorizationCodeVerifier* CodeVerifier;

	// Lobby tracking
	UPROPERTY()
	UDiscordLobbyHandle* CurrentLobbyHandle;

	UPROPERTY()
	FDiscordUniqueID CurrentLobbyId;

	// Member tracking
	UPROPERTY()
	TMap<FDiscordUniqueID, FLobbyMemberInfo> LobbyMembers;

	UPROPERTY()
	TMap<FDiscordUniqueID, UDiscordLobbyMemberHandle*> MemberHandles;

    FDiscordUniqueID LocalUserId;
public:
    FDiscordUniqueID GetLocalUserId() const { return LocalUserId; }
    // Helper functions
    void UpdateMemberInfo(FDiscordUniqueID UserId);
    void RemoveMemberInfo(FDiscordUniqueID UserId);
    void BroadcastMembersChanged();
};
