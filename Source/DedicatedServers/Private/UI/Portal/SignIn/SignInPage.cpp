#include "UI/Portal/SignIn/SignInPage.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Save/Portal/RememberSave.h"


constexpr const TCHAR* USignInPage::RememberSlot;
const int32 USignInPage::RememberUserIndex;

// Simple XOR obfuscation 
static uint32 SimpleKey(){ return 0xA3C51F2Bu; }
FString USignInPage::Obfuscate(const FString& In){
	TArray<TCHAR> A = In.GetCharArray();
	uint32 K = SimpleKey();
	for (int32 i=0;i<A.Num();++i){ A[i] = TCHAR(A[i] ^ (K + i*31)); }
	return FString(A.GetData());
}
FString USignInPage::Deobfuscate(const FString& In){ return Obfuscate(In); }

void USignInPage::NativeConstruct()
{
	Super::NativeConstruct();
	LoadRememberedCreds();
}

void USignInPage::LoadRememberedCreds()
{
	URememberSave* Data = Cast<URememberSave>(UGameplayStatics::LoadGameFromSlot(RememberSlot, RememberUserIndex));
	if (!Data)
	{
		if (CheckBox_RememberMe) CheckBox_RememberMe->SetIsChecked(true);
		return;
	}

	if (CheckBox_RememberMe) CheckBox_RememberMe->SetIsChecked(Data->bRememberMe);

	if (Data->bRememberMe)
	{
		if (TextBox_UserName && !Data->Username.IsEmpty())
			TextBox_UserName->SetText(FText::FromString(Data->Username));

		if (TextBox_Password && !Data->ObfuscatedPassword.IsEmpty())
			TextBox_Password->SetText(FText::FromString(Deobfuscate(Data->ObfuscatedPassword)));
	}
}

void USignInPage::SaveRememberedCreds()
{
	URememberSave* Data = Cast<URememberSave>(UGameplayStatics::LoadGameFromSlot(RememberSlot, RememberUserIndex));
	if (!Data)
	{
		Data = Cast<URememberSave>(UGameplayStatics::CreateSaveGameObject(URememberSave::StaticClass()));
	}

	const bool bRemember = CheckBox_RememberMe ? CheckBox_RememberMe->IsChecked() : true;
	Data->bRememberMe = bRemember;

	if (bRemember)
	{
		if (TextBox_UserName)
			Data->Username = TextBox_UserName->GetText().ToString();

		if (TextBox_Password)
			Data->ObfuscatedPassword = Obfuscate(TextBox_Password->GetText().ToString());
	}
	else
	{
		Data->Username.Empty();
		Data->ObfuscatedPassword.Empty();
	}

	UGameplayStatics::SaveGameToSlot(Data, RememberSlot, RememberUserIndex);
}

void USignInPage::ClearRememberedCreds()
{
	URememberSave* Data = Cast<URememberSave>(UGameplayStatics::LoadGameFromSlot(RememberSlot, RememberUserIndex));
	if (!Data)
	{
		Data = Cast<URememberSave>(UGameplayStatics::CreateSaveGameObject(URememberSave::StaticClass()));
	}
	Data->bRememberMe = false;
	Data->Username.Empty();
	Data->ObfuscatedPassword.Empty();
	UGameplayStatics::SaveGameToSlot(Data, RememberSlot, RememberUserIndex);

	if (CheckBox_RememberMe) CheckBox_RememberMe->SetIsChecked(false);
	if (TextBox_UserName)    TextBox_UserName->SetText(FText::GetEmpty());
	if (TextBox_Password)    TextBox_Password->SetText(FText::GetEmpty());
}

void USignInPage::UpdateStatusMessage(const FString& Message, bool bShouldResetWidgets)
{
	TextBlock_StatusMessage->SetText(FText::FromString(Message));
	if (bShouldResetWidgets)
	{
		Button_SignIn->SetIsEnabled(true);
	}
}


