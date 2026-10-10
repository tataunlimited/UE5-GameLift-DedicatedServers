#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "UI/HTTP/HTTPRequestTypes.h"
#include "DSLocalPlayerSubsystem.generated.h"

class IPortalManagement;
class UClientSaveManager;

/**
 * 
 */
UCLASS(Config=Game)
class DEDICATEDSERVERS_API UDSLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:

	void InitializeTokens(const FDSAuthenticationResult& AuthResult, TScriptInterface<IPortalManagement> PortalManagement);
	void SetRefreshTokenTimer();
	void UpdateTokens(const FString& AccessToken, const FString& IdToken);
	FDSAuthenticationResult GetAuthResult() const;
	bool IsSignedIn();

	FString Username{};
	FString Email{};
	FString Password{};
	bool bShouldLoadHomeAreaLevel;

	UFUNCTION(BlueprintPure, Category = "Player Session")
	UClientSaveManager* GetSaveManager() const { return ClientSaveManager; }

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
private:
	
	UPROPERTY()
	FDSAuthenticationResult AuthenticationResult;

	UPROPERTY()
	TScriptInterface<IPortalManagement> PortalManagementInterface;

	// 75% of an hour (the expiration time for AccessToken and IdToken)
	float TokenRefreshInterval = 2700.f; 

	FTimerHandle RefreshTimer;

	UPROPERTY()
	TObjectPtr<UClientSaveManager> ClientSaveManager;
};
