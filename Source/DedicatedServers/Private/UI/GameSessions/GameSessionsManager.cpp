#include "UI/GameSessions/GameSessionsManager.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Data/API/APIData.h"
#include "GameFramework/PlayerState.h"
#include "GameplayTags/DedicatedServerTags.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSLocalPlayerSubsystem.h"
#include "UI/HTTP/HTTPRequestTypes.h"

void UGameSessionsManager::JoinGameSession(const FString& Action)
{
	BroadcastJoinGameSessionMessage.Broadcast(TEXT("Searching for Game Session..."), false);
	check(APIData);

	CachedJoinAction = Action;
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UGameSessionsManager::FindOrCreateGameSession_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameSessionAPI::FindOrCreateGameSession);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");

	TMap<FString, FString> Params = {
		{TEXT("action"), Action},
		{TEXT("computeType"), bIsAnyWhereFleet? TEXT("ANYWHERE"): TEXT("EC2")},
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);

	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetDSLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem))
	{
		Request->SetHeader(TEXT("Authorization"), LocalPlayerSubsystem->GetAuthResult().AccessToken);
	}
	
	Request->ProcessRequest();
}

void UGameSessionsManager::FindOrCreateGameSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{

		if (ContainsError(JsonObject))
		{
			BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}
		FDSGameSession GameSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &GameSession);

		const FString GameSessionId = GameSession.GameSessionId;
		const FString GameSessionStatus = GameSession.Status;
		HandleGameSessionStatus(GameSessionStatus, GameSessionId);
	}
}

void UGameSessionsManager::HandleGameSessionStatus(const FString& Status, const FString& SessionId)
{
	if (Status.Equals(TEXT("ACTIVE"))){
		BroadcastJoinGameSessionMessage.Broadcast(TEXT("Found active Game Session. Creating a Player Session."),false);

		if (UDSLocalPlayerSubsystem* DSLocalPlayerSubsystem = GetDSLocalPlayerSubsystem(); IsValid(DSLocalPlayerSubsystem))
		{
			TryCreatePlayerSession(DSLocalPlayerSubsystem->Username, SessionId);
		}
	}
	else if (Status.Equals(TEXT("ACTIVATING")))
	{
		//FTimerDelegate CreateSessionDelegate;
		FTimerDelegate CreateSessionDelegate;
		CreateSessionDelegate.BindLambda([this]()
		{
			this->JoinGameSession(CachedJoinAction);
		});

		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (IsValid(LocalPlayerController))
		{
			LocalPlayerController->GetWorldTimerManager().SetTimer(CreateSessionTimer, CreateSessionDelegate, 0.5f, false);
		}
	}
	else
	{
		BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong,false);
	}
}

FString UGameSessionsManager::GetUniquePlayerId() const
{
	APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
	if (IsValid(LocalPlayerController))
	{
		APlayerState* LocalPlayerState = LocalPlayerController->GetPlayerState<APlayerState>();
		if (IsValid(LocalPlayerState) && LocalPlayerState->GetUniqueId().IsValid())
		{
			return TEXT("Player_") + FString::FromInt(LocalPlayerState->GetUniqueID());
		}
	}

	return FString();
}

void UGameSessionsManager::TryCreatePlayerSession(const FString& PlayerId, const FString& GameSessionId)
{
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UGameSessionsManager::CreatePlayerSession_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameSessionAPI::CreatePlayerSession);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");

	TMap<FString, FString> Params = {
		{TEXT("playerId"), PlayerId},
		{TEXT("gameSessionId"), GameSessionId},
		{TEXT("action"), CachedJoinAction},
		{TEXT("isAnyWhereFleet"), bIsAnyWhereFleet? TEXT("TRUE") : TEXT("FALSE")}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UGameSessionsManager::CreatePlayerSession_Response(FHttpRequestPtr Request, FHttpResponsePtr Response,
	bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{

		if (ContainsError(JsonObject))
		{
			BroadcastJoinGameSessionMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, true);
			return;
		}
		FDSPlayerSession PlayerSession;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &PlayerSession);

		PlayerSession.Dump();
		
		APlayerController* LocalPlayerController = GEngine->GetFirstLocalPlayerController(GetWorld());
		if (IsValid(LocalPlayerController))
		{
			FInputModeGameOnly InputModeData;
			LocalPlayerController->SetInputMode(InputModeData);
			LocalPlayerController->SetShowMouseCursor(false);
		}
		
		const FString Options = "?PlayerSessionId=" + PlayerSession.PlayerSessionId + "?Username=" + PlayerSession.PlayerId;
		//const FString Options = "?PlayerSessionId=" + PlayerSession.PlayerSessionId + "&Username=" + PlayerSession.PlayerId;

		const FString IpAndPort = PlayerSession.IpAddress + TEXT(":") + FString::FromInt(PlayerSession.Port);
		const FName Address(*IpAndPort);
		UGameplayStatics::OpenLevel(this, Address, true, Options);

	}
}