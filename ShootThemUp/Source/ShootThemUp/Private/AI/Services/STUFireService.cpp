// Shoot Them Up Game. All Rights Reserved.

#include "AI/Services/STUFireService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Components/STUAIWeaponComponent.h"
#include "Components/STUHealthComponent.h"
#include "STUUtils.h"

USTUFireService::USTUFireService()
{
	NodeName = "Fire";
}

void USTUFireService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const auto Controller{OwnerComp.GetAIOwner()};
	const auto Blackboard{OwnerComp.GetBlackboardComponent()};

	const bool IsHasAim{Blackboard && Blackboard->GetValueAsObject(EnemyActorKey.SelectedKeyName)};
	if (Controller)
	{
		const auto WeaponComponent{STUUtils::GetSTUPlayerComponent<USTUAIWeaponComponent>(Controller->GetPawn())};
		if (WeaponComponent)
		{
			IsHasAim ? WeaponComponent->OnStartFire() : WeaponComponent->OnStopFire();
		}
	}

	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
}
