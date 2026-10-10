using UnrealBuildTool;

public class DedicatedServers : ModuleRules
{
	public DedicatedServers(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine",
			"GameLiftServerSDK",
			"GameplayTags",
			"HTTP",
			"NetCore",
			"DeveloperSettings"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UMG",
			"Json",
			"JsonUtilities",
			"DiscordPartnerSDK"
		});

	}
}
