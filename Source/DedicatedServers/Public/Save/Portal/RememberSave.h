#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RememberSave.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API URememberSave : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY() bool bRememberMe = true;
	UPROPERTY() FString Username;
	UPROPERTY() FString ObfuscatedPassword;
};
