// Shoot Them Up Game. All Rights Reserved.

#include "Player/STUPlayerState.h"

DEFINE_LOG_CATEGORY_STATIC(LogSTUPlayerState, All, All);

void ASTUPlayerState::LogInfo()
{
	UE_LOG(LogSTUPlayerState, Display, TEXT("TeamID: %i, KillsNum: %i, DeathsNun: %i"), TeamData.ID, KillsNum, DeathsNum);
}