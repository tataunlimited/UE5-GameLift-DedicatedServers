#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

namespace DedicatedServerTags
{
	namespace GameSessionAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ListFleets)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(FindOrCreateGameSession)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(CreatePlayerSession)
	}

	namespace PortalAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SignUp)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConfirmSignUp)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SignIn)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SignOut)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ForgotPassword)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConfirmForgotPassword)
	}

	namespace GameStatsAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(RecordMatchStats)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(RetrieveMatchStats)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(UpdateLeaderboard)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(RetrieveLeaderboard)
	}

	namespace PlayerSaveAPI
	{
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(RequestUploadURL)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(RequestDownloadURL)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ConfirmSave)

		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ServerRequestUploadURL)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ServerRequestDownloadURL)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ServerConfirmSave)
		
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(SavePlayerData)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(LoadPlayerData)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ServerSavePlayerData)
		UE_DECLARE_GAMEPLAY_TAG_EXTERN(ServerLoadPlayerData)
	}
}