#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "DSPlayerState.generated.h"

class ACharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDisplayUpdated, FString, Username);

/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API ADSPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
	FString GetDisplayName();

	UFUNCTION(BlueprintCallable)
	void SetDisplayName(const FString& InUsername);
	
	UPROPERTY(BlueprintAssignable, Category="DisplayName")
	FOnDisplayUpdated OnDisplayNameUpdated;
	
	UPROPERTY(Replicated, BlueprintReadWrite, Category = "Player Properties")
	TSubclassOf<ACharacter> SelectedCharacterClass;
	


protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void CopyProperties(APlayerState* PlayerState) override;

	UPROPERTY(ReplicatedUsing = OnRep_DisplayName)
	FString DisplayName;

	UFUNCTION()
	void OnRep_DisplayName();
};

