// Shoot Them Up Game. All Rights Reserved.

#include "AI/STUAIController.h"
#include "AI/STUAICharacter.h"
#include "Components/STUAIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/STURespawnComponent.h"

ASTUAIController::ASTUAIController()
{
	STUAIPerceptionComponent = CreateDefaultSubobject<USTUAIPerceptionComponent>("STUAIPerceptionComponent");
	SetPerceptionComponent(*STUAIPerceptionComponent);

	RespawnComponent = CreateDefaultSubobject<USTURespawnComponent>("RespawnComponent");

	bWantsPlayerState = true;
}

void ASTUAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto AimActor{GetFocusOnActor()};
	SetFocus(AimActor);
}

void ASTUAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const auto STUCharacter{Cast<ASTUAICharacter>(InPawn)};
	if (STUCharacter)
	{
		RunBehaviorTree(STUCharacter->BehaviorTreeAsset);
	}
}

AActor* ASTUAIController::GetFocusOnActor() const
{
	return (!GetBlackboardComponent()) ? nullptr : //
			   Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(FocusOnKeyName));
}