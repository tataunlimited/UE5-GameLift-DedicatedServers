#include "UI/Portal/PortalHUD.h"
#include "UI/Portal/SignIn/SignInOverlay.h"
#include "UI/Portal/Dashboard/DashboardOverlay.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Player/DSLocalPlayerSubsystem.h"
#include "Save/ClientSaveManager.h"
#include "UI/Portal/PortalManager.h"

void APortalHUD::BeginPlay()
{
	Super::BeginPlay();
	check(PortalManagerClass);
	
	PortalManager = NewObject<UPortalManager>(this, PortalManagerClass);

	APlayerController* OwningPlayerController = GetOwningPlayerController();
	FInputModeGameAndUI InputModeData;
	OwningPlayerController->SetInputMode(InputModeData);
	OwningPlayerController->SetShowMouseCursor(true);
	
	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = PortalManager->GetDSLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem) && LocalPlayerSubsystem->IsSignedIn())
	{
		bRefreshingTokensFromHUD = true;
		PortalManager->RefreshTokens(LocalPlayerSubsystem->GetAuthResult().RefreshToken);
		ShowLoadingWidget(true);
		return;
	}

	ShowSignInOverlay(true);
}



void APortalHUD::OnSignIn()
{
	if (IsValid(SignInOverlay))
	{
		SignInOverlay->OnSignInSucceeded();
		SignInOverlay->RemoveFromParent();
	}

	LoadSavedData();
	ShowLoadingWidget(true);
	
}

void APortalHUD::OnSignOut()
{
	ShowDashboardOverlay(false);
	ShowSignInOverlay(true);
}

void APortalHUD::OnTokensRefreshed(bool bIsSuccessful)
{
	if (!bRefreshingTokensFromHUD)
	{
		return;
	}
	bRefreshingTokensFromHUD = false;
	
	if (bIsSuccessful)
	{
		LoadSavedData();
	}
	else
	{
		ShowSignInOverlay(true);
		ShowLoadingWidget(false);
	}
}
void APortalHUD::ShowLoadingWidget(bool bShow)
{
	if (bShow)
	{
		APlayerController* OwningPlayerController = GetOwningPlayerController();
		LoadingWidget = CreateWidget<UUserWidget>(OwningPlayerController, LoadingWidgetClass);
		if (IsValid(LoadingWidget))
		{
			LoadingWidget->AddToViewport();
		}
	}else
	{
		if (IsValid(LoadingWidget))
			LoadingWidget->RemoveFromParent();
	}
}

void APortalHUD::ShowDashboardOverlay(bool bShow)
{
	if (bShow)
	{
		APlayerController* OwningPlayerController = GetOwningPlayerController();
		DashboardOverlay = CreateWidget<UDashboardOverlay>(OwningPlayerController, DashboardOverlayClass);
		if (IsValid(DashboardOverlay))
		{
			DashboardOverlay->AddToViewport();
		}
	}else
	{
		if (IsValid(DashboardOverlay))
		{
			DashboardOverlay->RemoveFromParent();
		}
	}
}

void APortalHUD::ShowSignInOverlay(bool bShow)
{
	if (bShow)
	{
		APlayerController* OwningPlayerController = GetOwningPlayerController();
		SignInOverlay = CreateWidget<USignInOverlay>(OwningPlayerController, SignInOverlayClass);
		if (IsValid(SignInOverlay))
		{
			SignInOverlay->AddToViewport();
		}
	}
	else
	{
		if (IsValid(SignInOverlay))
		{
			SignInOverlay->RemoveFromParent();
		}
	}
}


void APortalHUD::LoadSavedData()
{
	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = PortalManager->GetDSLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem))
	{
		LocalPlayerSubsystem->GetSaveManager()->LoadPlayerStateFromS3();
		LocalPlayerSubsystem->GetSaveManager()->OnLoadFromS3Complete.AddDynamic(this, &APortalHUD::OnSavedDataLoaded);
	}
}

void APortalHUD::OnSavedDataLoaded(bool bWasSuccessful, USaveGame* LoadedHomeAreaData, USaveGame* LoadedPlayerData)
{
	ShowLoadingWidget(false);
	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = PortalManager->GetDSLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem))
	{
		LocalPlayerSubsystem->GetSaveManager()->OnLoadFromS3Complete.RemoveDynamic(this, &APortalHUD::OnSavedDataLoaded);
		if (LocalPlayerSubsystem->bShouldLoadHomeAreaLevel)
		{
			LocalPlayerSubsystem->bShouldLoadHomeAreaLevel = false;
			UGameplayStatics::OpenLevelBySoftObjectPtr(this, HomeAreaMap);
			return;
		}
	}
	ShowDashboardOverlay(true);
}

