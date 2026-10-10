#pragma once

#include "Net/Serialization/FastArraySerializer.h"
#include "LobbyPlayerInfo.generated.h"

USTRUCT(BlueprintType)
struct FLobbyPlayerInfo: public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLobbyPlayerInfo() {}
	FLobbyPlayerInfo(const FString& Name) : Username(Name) {}

	UPROPERTY(BlueprintReadWrite)
	FString Username{};

	UPROPERTY(BlueprintReadWrite)
	bool bIsReady = false; 

	// Discord voice status
	UPROPERTY(BlueprintReadWrite)
	bool bVoiceConnected = false;

	UPROPERTY(BlueprintReadWrite)
	bool bIsSpeaking = false;
};

USTRUCT()
struct FLobbyPlayerInfoArray : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FLobbyPlayerInfo> Players;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
	{
		return FastArrayDeltaSerialize<FLobbyPlayerInfo, FLobbyPlayerInfoArray>(Players, DeltaParams, *this);
	}
	
	void SetPlayerReadyState(const FString& Username, bool bReady);
	void AddPlayer(const FLobbyPlayerInfo& NewPlayerInfo);
	void RemovePlayer(const FString& Username);
	void SetPlayerVoiceStatus(const FString& Username, bool bVoiceConnected, bool bIsSpeaking);
};

template<>
struct TStructOpsTypeTraits<FLobbyPlayerInfoArray> : public TStructOpsTypeTraitsBase2<FLobbyPlayerInfoArray>
{
	enum
	{
		WithNetDeltaSerializer = true,
	};
};
