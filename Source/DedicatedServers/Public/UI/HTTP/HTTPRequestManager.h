#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HTTPRequestManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FAPIStatusMessage, const FString&, StatusMessage, bool, bShouldResetWidgets);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAPIRequestSucceeded);

class UAPIData;
class UDSLocalPlayerSubsystem;
/**
 * 
 */
UCLASS(Blueprintable)
class DEDICATEDSERVERS_API UHTTPRequestManager : public UObject
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "HTTP Request Manager")
	UDSLocalPlayerSubsystem* GetDSLocalPlayerSubsystem() const;
	
protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAPIData> APIData;

	bool ContainsError(TSharedPtr<FJsonObject> JsonObject);
	void DumpMetaData(TSharedPtr<FJsonObject> JsonObject);

	FString SerializeJsonContent(const TMap<FString, FString>& Params);
};
