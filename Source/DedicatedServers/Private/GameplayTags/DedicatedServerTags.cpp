#include "GamePlayTags/DedicatedServerTags.h"

namespace DedicatedServerTags
{
	namespace GameSessionAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ListFleets, "DedicatedServerTags.GameSessionAPI.ListFleets", "List Fleets resource on the GameSessions API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(FindOrCreateGameSession, "DedicatedServerTags.GameSessionAPI.FindOrCreateGameSession", "Retrieves an ACTIVE Game Session, creating one if one does not exist on the GameSessions API ")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(CreatePlayerSession, "DedicatedServerTags.GameSessionAPI.CreatePlayerSession", "Creates a new Player Session on the GameSessions API ")
	}

	namespace PortalAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SignUp, "DedicatedServerTags.PortalAPI.SignUp", "Creates a new user in the Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ConfirmSignUp, "DedicatedServerTags.PortalAPI.ConfirmSignUp", "Confirms a new user in the Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SignIn, "DedicatedServerTags.PortalAPI.SignIn", "Retrieves Access Token, Id Token, and Refresh Token in the Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SignOut, "DedicatedServerTags.PortalAPI.SignOut", "Signs user out and validates tokens in the Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ForgotPassword, "DedicatedServerTags.PortalAPI.ForgotPassword", "Requests forgot password in the Portal API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ConfirmForgotPassword, "DedicatedServerTags.PortalAPI.ConfirmForgotPassword", "Confirms a new password in the Portal API")
	}

	namespace GameStatsAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RecordMatchStats, "DedicatedServerTags.GameStatsAPI.RecordMatchStats", "Records the stats of a match in Game Stats API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RetrieveMatchStats, "DedicatedServerTags.GameStatsAPI.RetrieveMatchStats", "Get the match stats from the Players table in Game Stats API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(UpdateLeaderboard, "DedicatedServerTags.GameStatsAPI.UpdateLeaderboard", "Updates the Leaderboard table in Game Stats API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RetrieveLeaderboard, "DedicatedServerTags.GameStatsAPI.RetrieveLeaderboard", "Retrieves the Leaderboard in Game Stats API")
	}

	namespace PlayerSaveAPI
	{
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestUploadURL, "DedicatedServerTags.PlayerSaveAPI.RequestUploadURL", "Requests upload urls in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(RequestDownloadURL, "DedicatedServerTags.PlayerSaveAPI.RequestDownloadURL", "Requests download urls in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ConfirmSave, "DedicatedServerTags.PlayerSaveAPI.ConfirmSave", "Confirms Save in the Player Save API")

		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ServerRequestUploadURL, "DedicatedServerTags.PlayerSaveAPI.ServerRequestUploadURL", "Requests upload urls in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ServerRequestDownloadURL, "DedicatedServerTags.PlayerSaveAPI.ServerRequestDownloadURL", "Requests download urls in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ServerConfirmSave, "DedicatedServerTags.PlayerSaveAPI.ServerConfirmSave", "Confirms Save in the Player Save API")
		
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(SavePlayerData, "DedicatedServerTags.PlayerSaveAPI.SavePlayerData", "Saves Player and HomeArea Data in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(LoadPlayerData, "DedicatedServerTags.PlayerSaveAPI.LoadPlayerData", "Loads Player and HomeArea Data in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ServerSavePlayerData, "DedicatedServerTags.PlayerSaveAPI.ServerSavePlayerData", "Loads Player Data in the Player Save API")
		UE_DEFINE_GAMEPLAY_TAG_COMMENT(ServerLoadPlayerData, "DedicatedServerTags.PlayerSaveAPI.ServerLoadPlayerData", "Saves Player Data in the Player Save API")
	}
}