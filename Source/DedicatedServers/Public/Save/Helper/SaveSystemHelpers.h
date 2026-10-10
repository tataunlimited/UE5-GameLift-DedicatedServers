#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameFramework/SaveGame.h" 
#include "SaveSystemHelpers.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API USaveSystemHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	
	UFUNCTION(BlueprintCallable, Category = "Custom Save System")
	static TArray<uint8> SerializeSaveGameObject(USaveGame* InSaveGame);


	UFUNCTION(BlueprintCallable, Category = "Custom Save System")
	static USaveGame* DeserializeSaveGameObject(TSubclassOf<USaveGame> SaveGameClass, const TArray<uint8>& InData);
};
