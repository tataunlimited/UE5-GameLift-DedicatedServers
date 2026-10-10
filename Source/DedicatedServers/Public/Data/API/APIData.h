#pragma once

#include "CoreMinimal.h"
#include "GameplayTags/DedicatedServerTags.h"
#include "Engine/DataAsset.h"
#include "APIData.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API UAPIData : public UDataAsset
{
	GENERATED_BODY()

public:

	FString GetAPIEndpoint(const FGameplayTag& APIEndpoint);

protected:

	// Name of this API - for labeling in the Data Asset; this is not used by any code.
	UPROPERTY(EditDefaultsOnly)
	FString Name;

	UPROPERTY(EditDefaultsOnly)
	FString InvokeURL;
	
	UPROPERTY(EditDefaultsOnly)
	FString Stage;

	UPROPERTY(EditDefaultsOnly)
	TMap<FGameplayTag, FString> Resources;

	
};
