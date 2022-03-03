// Shoot Them Up Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "STUCoreTypes.h"
#include "Templates/UnrealTemplate.h"
#include "STUPlayerState.generated.h"

UCLASS()
class SHOOTTHEMUP_API ASTUPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void AddKill() { ++KillsNum; }
	int32 GetKillsNum() { return KillsNum; }

	void AddDeath() { ++DeathsNum; }
	int32 GetDeathsNum() { return DeathsNum; }

	void SetTeamData(const FTeamData& Data) { TeamData = Data; }
	void SetTeamData(FTeamData&& Data) { Swap(TeamData, Data); };
	FTeamData GetTeamData() const { return TeamData; }

	void LogInfo();

private:
	FTeamData TeamData;

	int32 KillsNum{0};
	int32 DeathsNum{0};
};
