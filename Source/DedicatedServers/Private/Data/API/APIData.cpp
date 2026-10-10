#include "Data/API/APIData.h"

FString UAPIData::GetAPIEndpoint(const FGameplayTag& APIEndpoint)
{
	FString ResourceName = Resources.FindChecked(APIEndpoint);

	return InvokeURL + "/" + Stage + "/" + ResourceName;
}
