#include "Save/Helper/SaveSystemHelpers.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

TArray<uint8> USaveSystemHelpers::SerializeSaveGameObject(USaveGame* InSaveGame)
{
	// Create an array to hold the serialized data
	TArray<uint8> SaveData;

	if (!InSaveGame)
	{
		UE_LOG(LogTemp, Warning, TEXT("SerializeSaveGameObject: InSaveGame was null."));
		return SaveData;
	}

	// FMemoryWriter is an archive that writes data to our byte array
	FMemoryWriter MemoryWriter(SaveData, true);

	// This special archive wrapper helps serialize UObjects and their properties correctly
	FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, false);

	// This is the command that tells the SaveGame object to write its data into the archive
	InSaveGame->Serialize(Archive);

	return SaveData;
}


// --- Implementation for the Deserialize function ---
USaveGame* USaveSystemHelpers::DeserializeSaveGameObject(TSubclassOf<USaveGame> SaveGameClass, const TArray<uint8>& InData)
{
	if (!SaveGameClass || InData.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("DeserializeSaveGameObject: Invalid SaveGameClass or empty InData."));
		return nullptr;
	}

	// First, create a new object of the specified SaveGame class
	USaveGame* LoadedGame = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass);
    
	// FMemoryReader is the opposite of FMemoryWriter; it reads data from a byte array
	FMemoryReader MemoryReader(InData, true);

	// Create the archive with the reader
	FObjectAndNameAsStringProxyArchive Archive(MemoryReader, false);
    
	// Tell the new, empty SaveGame object to fill itself with data from the archive
	LoadedGame->Serialize(Archive);

	return LoadedGame;
}