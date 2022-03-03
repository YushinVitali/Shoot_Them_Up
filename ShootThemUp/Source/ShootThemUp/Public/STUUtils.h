// Shoot Them Up Game. All Rights Reserved.

#pragma once

#include "Player/STUPlayerState.h"

class AActor;

class STUUtils
{
public:
	template <typename T> //
	static T* GetSTUPlayerComponent(AActor* PlayerPawn)
	{
		if (!PlayerPawn)
		{
			return nullptr;
		}

		const auto Component{PlayerPawn->GetComponentByClass(T::StaticClass())};
		return Cast<T>(Component);
	}

	static bool AreEnemies(AController* Controller1, AController* Controller2)
	{
		if (!Controller1 || !Controller2 || Controller1 == Controller2)
		{
			return false;
		}

		const auto PlayerState1{Cast<ASTUPlayerState>(Controller1->PlayerState)};
		const auto PlayerState2{Cast<ASTUPlayerState>(Controller2->PlayerState)};
		if (!PlayerState1 || !PlayerState2)
		{
			return false;
		}

		return PlayerState1 && PlayerState2 && PlayerState1->GetTeamData().ID != PlayerState2->GetTeamData().ID;
	}

	static FText GetTextFromInt(int32 Number)
	{
		return FText::FromString(FString::FromInt(Number));
	}
};
