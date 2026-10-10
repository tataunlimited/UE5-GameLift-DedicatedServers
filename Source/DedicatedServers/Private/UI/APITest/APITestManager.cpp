#include "UI/APITest/APITestManager.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "Gameplaytags/DedicatedServerTags.h"
#include "Data/API/APIData.h"
#include "DedicatedServers/DedicatedServers.h"
#include "Interfaces/IHttpResponse.h"
#include "UI/HTTP/HTTPRequestTypes.h"

void UAPITestManager::ListFleets()
{
	check(APIData);
	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &UAPITestManager::ListFleets_Response);
	const FString APIUrl = APIData->GetAPIEndpoint(DedicatedServerTags::GameSessionAPI::ListFleets);
	Request->SetURL(APIUrl);
	Request->SetVerb(TEXT("GET"));
	Request->SetHeader("Content-Type", "application/json");
	Request->ProcessRequest();
	
}

void UAPITestManager::ListFleets_Response(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSucceeded)
{
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Red, "List Fleets Response Received");

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
	{

		if (ContainsError(JsonObject))
		{
			OnListFleetsResponseReceived.Broadcast(FDSListFleetsResponse(), false);
			return;
		}
		DumpMetaData(JsonObject);

		FDSListFleetsResponse ListFleetsResponse;
		FJsonObjectConverter::JsonObjectToUStruct(JsonObject.ToSharedRef(), &ListFleetsResponse);
		ListFleetsResponse.Dump();

		OnListFleetsResponseReceived.Broadcast(ListFleetsResponse, true);

	}
}
