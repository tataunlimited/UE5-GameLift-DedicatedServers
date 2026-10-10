#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GameFramework/SaveGame.h"
#include "UI/Interfaces/HUDManagement.h"
#include "PortalHUD.generated.h"

class UDashboardOverlay;
class USignInOverlay;
class UPortalManager;
class UUserWidget;
/**
 * 
 */
UCLASS()
class DEDICATEDSERVERS_API APortalHUD : public AHUD, public IHUDManagement
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPortalManager> PortalManagerClass;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USignInOverlay> SignInOverlayClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadingWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDashboardOverlay> DashboardOverlayClass;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> HomeAreaMap;

	// IHUDManagement
	virtual void OnSignIn() override;
	virtual void OnSignOut() override;
	virtual void OnTokensRefreshed(bool bIsSuccessful) override;

	

protected:
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	TObjectPtr<USignInOverlay> SignInOverlay;

	UPROPERTY()
	TObjectPtr<UDashboardOverlay> DashboardOverlay;

	UPROPERTY()
	TObjectPtr<UUserWidget> LoadingWidget;

	UPROPERTY()
	TObjectPtr<UPortalManager> PortalManager;

	void ShowLoadingWidget(bool bShow);

	void ShowDashboardOverlay(bool bShow);

	void ShowSignInOverlay(bool bShow);

	UFUNCTION()
	void OnSavedDataLoaded(bool bWasSuccessful, USaveGame* LoadedHomeAreaData, USaveGame* LoadedPlayerData);
	
	void LoadSavedData();

	bool bRefreshingTokensFromHUD;
};
