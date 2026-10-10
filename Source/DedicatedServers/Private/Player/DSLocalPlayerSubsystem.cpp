
#include "Player/DSLocalPlayerSubsystem.h"
#include "Save/ClientSaveManager.h"
#include "Settings/DSGameSettings.h"
#include "UI/Portal/Interfaces/PortalManagement.h"


void ::UDSLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	const UDSGameSettings* GameSettings = GetDefault<UDSGameSettings>();

	if (GameSettings && !GameSettings->ClientSaveManagerClass.IsNull())
	{
		if (const UClass* SaveManagerClass = GameSettings->ClientSaveManagerClass.LoadSynchronous())
		{
			ClientSaveManager = NewObject<UClientSaveManager>(this, SaveManagerClass);
			ClientSaveManager->Initialize();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ClientSaveManagerClass is not set in Project Settings!"));
	}
}


void UDSLocalPlayerSubsystem::InitializeTokens(const FDSAuthenticationResult& AuthResult,TScriptInterface<IPortalManagement> PortalManagement)
{
	AuthenticationResult = AuthResult;
	PortalManagementInterface = PortalManagement;
	SetRefreshTokenTimer();
}

void UDSLocalPlayerSubsystem::SetRefreshTokenTimer()
{
	UWorld* World = GetWorld();
	if (IsValid(World) && IsValid(PortalManagementInterface.GetObject()))
	{
		FTimerDelegate RefreshTokenDelegate;
		RefreshTokenDelegate.BindLambda([this]() {PortalManagementInterface->RefreshTokens(AuthenticationResult.RefreshToken); });
		World->GetTimerManager().SetTimer(RefreshTimer, RefreshTokenDelegate, TokenRefreshInterval, false);
	}
}

void UDSLocalPlayerSubsystem::UpdateTokens(const FString& AccessToken, const FString& IdToken)
{
	AuthenticationResult.AccessToken = AccessToken;
	AuthenticationResult.IdToken = IdToken;
	AuthenticationResult.Dump();
	SetRefreshTokenTimer();
}

FDSAuthenticationResult UDSLocalPlayerSubsystem::GetAuthResult() const
{
	return AuthenticationResult;
}

bool UDSLocalPlayerSubsystem::IsSignedIn()
{
	if (AuthenticationResult.AccessToken.IsEmpty())
	{
		return false;
	}
	return true;
}
