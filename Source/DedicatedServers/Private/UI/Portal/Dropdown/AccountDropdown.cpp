#include "UI/Portal/Dropdown/AccountDropdown.h"
#include "Components/TextBlock.h"
#include "Player/DSLocalPlayerSubsystem.h"

void UAccountDropdown::NativeConstruct()
{
	Super::NativeConstruct();
	UDSLocalPlayerSubsystem* LocalPlayerSubsystem = GetLocalPlayerSubsystem();
	if (IsValid(LocalPlayerSubsystem))
	{
		TextBlock_ButtonText->SetText(FText::FromString(LocalPlayerSubsystem->Username));
	}
}

UDSLocalPlayerSubsystem* UAccountDropdown::GetLocalPlayerSubsystem() const
{
	APlayerController* PlayerController =  GetOwningPlayer();
	if (IsValid(PlayerController))
	{
		UDSLocalPlayerSubsystem* LocalPlayerSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UDSLocalPlayerSubsystem>();
		if (IsValid(LocalPlayerSubsystem))
		{
			return LocalPlayerSubsystem;
		}
	}
	return nullptr;
}
