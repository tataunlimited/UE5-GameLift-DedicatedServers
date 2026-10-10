#include "UI/GameStats/GameStatsManager.h"

#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Data/API/APIData.h"
#include "DedicatedServers/DedicatedServers.h"
#include "Interfaces/IHttpRequest.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "GameplayTags/DedicatedServerTags.h"
#include "Interfaces/IHttpResponse.h"
#include "Player/DSLocalPlayerSubsystem.h"

void UGameStatsManager::RecordMatchStats(const FDSRecordMatchStatsInput& RecordMatchStatsInput)
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(FDSRecordMatchStatsInput::StaticStruct(), &RecordMatchStatsInput, JsonString);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameStatsAPI::RecordMatchStats);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::RecordMatchStats_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(JsonString);
	Request->ProcessRequest();
}

void UGameStatsManager::RecordMatchStats_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		UE_LOG(LogDedicatedServers, Error, TEXT("RecordMatchStats failed."))
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		ContainsError(JsonObject);
	}
}

void UGameStatsManager::RetrieveMatchStats()
{
	RetrieveMatchStatsStatusMessage.Broadcast(TEXT("Retrieving match stats..."), false);
	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetDSLocalPlayerSubsystem();
	if (!IsValid(LocalPlayerSubsystem)) return;
	check(APIData);
	
	const FString AccessToken = LocalPlayerSubsystem->GetAuthResult().AccessToken;
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameStatsAPI::RetrieveMatchStats);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::RetrieveMatchStats_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");

	TMap<FString, FString> Params = {
		{TEXT("accessToken"), AccessToken}
	};
	const FString Content = SerializeJsonContent(Params);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UGameStatsManager::RetrieveMatchStats_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		OnRetrieveMatchStatsResponseReceived.Broadcast(FDSRetrieveMatchStatsResponse());
		RetrieveMatchStatsStatusMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, false);
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			OnRetrieveMatchStatsResponseReceived.Broadcast(FDSRetrieveMatchStatsResponse());
			RetrieveMatchStatsStatusMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, false);
			return;
		}

		FDSRetrieveMatchStatsResponse RetrieveMatchStatsResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &RetrieveMatchStatsResponse);
		RetrieveMatchStatsResponse.Dump();

		OnRetrieveMatchStatsResponseReceived.Broadcast(RetrieveMatchStatsResponse);
		RetrieveMatchStatsStatusMessage.Broadcast(TEXT(""), false);

	}
}
void UGameStatsManager::UpdateLeaderboard(const TArray<FString>& WinnerUsernames)
{
	check(APIData);
	
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameStatsAPI::UpdateLeaderboard);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::UpdateLeaderboard_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader("Content-Type", "application/json");

	TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
	TArray<TSharedPtr<FJsonValue>> PlayerIdJsonArray;

	for (const auto& Username : WinnerUsernames)
	{
		PlayerIdJsonArray.Add(MakeShareable(new FJsonValueString(Username)));
	}
	JsonObject->SetArrayField(TEXT("playerIds"), PlayerIdJsonArray);
	FString Content;
	TSharedRef<TJsonWriter<>> JsonWriter = TJsonWriterFactory<>::Create(&Content);
	FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
	
	Request->SetContentAsString(Content);
	Request->ProcessRequest();
}

void UGameStatsManager::UpdateLeaderboard_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		return;
	}
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			return;
		}
	}
	OnUpdateLeaderboardSucceeded.Broadcast();
}

void UGameStatsManager::RetrieveLeaderboard()
{
	RetrieveLeaderboardStatusMessage.Broadcast(TEXT("Retrieving leaderboard..."), false);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameStatsAPI::RetrieveLeaderboard);
	Request->OnProcessRequestComplete().BindUObject(this, &UGameStatsManager::RetrieveLeaderboard_Response);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader("Content-Type", "application/json");
	Request->ProcessRequest();
}

void UGameStatsManager::RetrieveLeaderboard_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		RetrieveLeaderboardStatusMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, false);
		UE_LOG(LogDedicatedServers, Error, TEXT("RetrieveLeaderboard failed."))
		return;
	}

	TArray<FDSLeaderboardItem> LeaderboardItems;
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{
		if (ContainsError(JsonObject))
		{
			RetrieveLeaderboardStatusMessage.Broadcast(HTTPStatusMessages::SomethingWentWrong, false);
			return;
		}
		const TArray<TSharedPtr<FJsonValue>>* LeaderboardJsonArray;
		if (JsonObject->TryGetArrayField(TEXT("Leaderboard"), LeaderboardJsonArray))
		{
			for (const TSharedPtr<FJsonValue>& ItemValue : *LeaderboardJsonArray)
			{
				TSharedPtr<FJsonObject> ItemObject = ItemValue->AsObject();
				if (ItemObject.IsValid())
				{
					FDSLeaderboardItem Item;
					if (FJsonObjectConverter::JsonObjectToUStruct(ItemObject.ToSharedRef(), &Item))
					{
						LeaderboardItems.Add(Item);
					}
					else
					{
						UE_LOG(LogDedicatedServers, Error, TEXT("Failed to parse leaderboard item."))
					}
				}
			}
		}
	}
	OnRetrieveLeaderboard.Broadcast(LeaderboardItems);
	RetrieveLeaderboardStatusMessage.Broadcast(TEXT(""), false);

}

