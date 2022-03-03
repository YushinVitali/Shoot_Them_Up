// Shoot Them Up Game. All Rights Reserved.

#include "UI/STUGameOverWidget.h"
#include "STUGameModeBase.h"
#include "Player/STUPlayerState.h"
#include "UI/STUPlayerStatRowWidget.h"
#include "Components/VerticalBox.h"
#include "STUUtils.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

void USTUGameOverWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (GetWorld())
	{
		const auto GameMode{Cast<ASTUGameModeBase>(GetWorld()->GetAuthGameMode())};
		if (GameMode)
		{
			GameMode->OneMatchStateChanged.AddUObject(this, &USTUGameOverWidget::OnMatchStateChanged);
		}
	}

	if (ResetLevelButton)
	{
		ResetLevelButton->OnClicked.AddDynamic(this, &USTUGameOverWidget::OnResetLevel);
	}
}

void USTUGameOverWidget::OnMatchStateChanged(ESTUMatchState State)
{
	if (State == ESTUMatchState::GameOver)
	{
		UpdatePlayersStat();
	}
}

void USTUGameOverWidget::UpdatePlayersStat()
{
	if (!GetWorld() || !PlayerStatBox)
	{
		return;
	}

	PlayerStatBox->ClearChildren();

	for (auto It = GetWorld()->GetControllerIterator(); It; ++It)
	{
		const auto Controller{It->Get()};
		if (!Controller)
		{
			continue;
		}

		const auto PlayerStat{Cast<ASTUPlayerState>(Controller->PlayerState)};
		if (!PlayerStat)
		{
			continue;
		}

		const auto PlayerStatRowWidget{CreateWidget<USTUPlayerStatRowWidget>(GetWorld(), PlayerStatRowWidgetClass)};
		if (!PlayerStatRowWidget)
		{
			continue;
		}

		PlayerStatRowWidget->SetPlayerName(FText::FromString(PlayerStat->GetPlayerName()));
		PlayerStatRowWidget->SetKills(STUUtils::GetTextFromInt(PlayerStat->GetKillsNum()));
		PlayerStatRowWidget->SetDeaths(STUUtils::GetTextFromInt(PlayerStat->GetDeathsNum()));
		PlayerStatRowWidget->SetTeam(STUUtils::GetTextFromInt(PlayerStat->GetTeamData().ID));

		PlayerStatRowWidget->SetPlayerIndicatorVisibility(Controller->IsPlayerController());

		PlayerStatRowWidget->SetTeamColor(PlayerStat->GetTeamData().Color);

		PlayerStatBox->AddChild(PlayerStatRowWidget);
	}
}

void USTUGameOverWidget::OnResetLevel()
{
	const FString CurrentLevelName{UGameplayStatics::GetCurrentLevelName(this)};
	UGameplayStatics::OpenLevel(this, FName(CurrentLevelName));
}